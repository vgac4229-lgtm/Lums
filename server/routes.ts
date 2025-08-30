import express from 'express';
import { body, validationResult } from 'express-validator';
import rateLimit from 'express-rate-limit';
import { logger } from './logger';
import { swaggerUi, swaggerSpec, swaggerOptions } from './api-docs';

const router = express.Router();

// Rate limiting plus stricte
const apiLimiter = rateLimit({
  windowMs: 15 * 60 * 1000, // 15 minutes
  max: 100, // 100 requêtes par IP
  message: {
    error: 'Too many requests from this IP, please try again later.',
    retry_after: '15 minutes'
  },
  standardHeaders: true,
  legacyHeaders: false,
});

// Middleware de validation d'erreurs
const handleValidationErrors = (req: express.Request, res: express.Response, next: express.NextFunction) => {
  const errors = validationResult(req);
  if (!errors.isEmpty()) {
    logger.log('warn', 'Validation errors', { 
      errors: errors.array(),
      url: req.url,
      body: req.body,
      op: 'validation_error'
    });
    return res.status(400).json({
      error: 'Validation failed',
      details: errors.array()
    });
  }
  next();
};

// Règles de validation strictes
const validateBitString = [
  body('inputBits')
    .isString()
    .withMessage('inputBits must be a string')
    .matches(/^[01]+$/)
    .withMessage('inputBits must contain only 0s and 1s')
    .isLength({ min: 1, max: 1000 })
    .withMessage('inputBits must be between 1 and 1000 characters'),
];

const validateVoraxOperation = [
  body('type')
    .isIn(['fusion', 'split', 'cycle', 'flow'])
    .withMessage('type must be one of: fusion, split, cycle, flow'),
  body('groups')
    .isArray({ min: 1 })
    .withMessage('groups must be a non-empty array'),
  body('groups.*.lums')
    .isArray()
    .withMessage('each group must have a lums array'),
];

/**
 * @swagger
 * /api/convert/bit-to-lum:
 *   post:
 *     tags: [Conversion]
 *     summary: Convert binary string to LUM representation
 *     description: Transforms a binary string into an array of LUM structures with spatial positioning
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             type: object
 *             required: [inputBits]
 *             properties:
 *               inputBits:
 *                 type: string
 *                 pattern: ^[01]+$
 *                 minLength: 1
 *                 maxLength: 1000
 *                 example: "110100101"
 *                 description: Binary string to convert
 *     responses:
 *       200:
 *         description: Conversion successful
 *         content:
 *           application/json:
 *             schema:
 *               type: object
 *               properties:
 *                 lums:
 *                   type: array
 *                   items:
 *                     $ref: '#/components/schemas/LUM'
 *                 conversion_stats:
 *                   type: object
 *                   properties:
 *                     input_length:
 *                       type: integer
 *                     output_count:
 *                       type: integer
 *                     conversion_time_ms:
 *                       type: number
 *       400:
 *         description: Validation error
 *         content:
 *           application/json:
 *             schema:
 *               $ref: '#/components/schemas/ApiError'
 */
router.post('/convert/bit-to-lum', apiLimiter, validateBitString, handleValidationErrors, (req, res) => {
  const startTime = process.hrtime.bigint();
  const { inputBits } = req.body;

  try {
    // Conversion bit → LUM avec génération d'IDs uniques
    const lums = inputBits.split('').map((bit: string, index: number) => ({
      presence: parseInt(bit),
      structureType: 'linear',
      position: { x: index * 20, y: 0 },
      lum_id: `L-${logger.runId || 'system'}-${index.toString().padStart(6, '0')}`,
      metadata: {
        bit_position: index,
        source_bit: bit,
        conversion_timestamp: new Date().toISOString()
      }
    }));

    const endTime = process.hrtime.bigint();
    const conversionTimeMs = Number((endTime - startTime) / BigInt(1000000));

    // Log conversion avec détails complets
    logger.logConversion(inputBits, lums, {
      conversion_time_ms: conversionTimeMs,
      endpoint: '/convert/bit-to-lum',
      client_ip: req.ip
    });

    res.json({
      lums,
      conversion_stats: {
        input_length: inputBits.length,
        output_count: lums.length,
        conversion_time_ms: conversionTimeMs,
        run_id: logger.runId,
        conservation_valid: inputBits.length === lums.length
      }
    });

  } catch (error) {
    logger.log('error', 'Bit conversion failed', { 
      error: error instanceof Error ? error.message : 'Unknown error',
      input: inputBits,
      op: 'conversion_error'
    });
    res.status(500).json({ error: 'Conversion failed' });
  }
});

/**
 * @swagger
 * /api/convert/lum-to-bit:
 *   post:
 *     tags: [Conversion]
 *     summary: Convert LUM array back to binary string
 *     description: Transforms an array of LUM structures back into a binary string
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             type: object
 *             required: [lums]
 *             properties:
 *               lums:
 *                 type: array
 *                 items:
 *                   $ref: '#/components/schemas/LUM'
 *     responses:
 *       200:
 *         description: Reverse conversion successful
 *       400:
 *         description: Validation error
 */
router.post('/convert/lum-to-bit', apiLimiter, (req, res) => {
  const { lums } = req.body;

  try {
    if (!Array.isArray(lums)) {
      return res.status(400).json({ error: 'lums must be an array' });
    }

    const resultBits = lums.map((lum: any) => lum.presence.toString()).join('');

    logger.log('info', 'LUM to bit conversion', {
      op: 'lum_to_bit_conversion',
      input_count: lums.length,
      output_length: resultBits.length,
      conservation_valid: lums.length === resultBits.length
    });

    res.json({
      outputBits: resultBits,
      conversion_stats: {
        input_count: lums.length,
        output_length: resultBits.length,
        conservation_valid: lums.length === resultBits.length,
        run_id: logger.runId
      }
    });

  } catch (error) {
    logger.log('error', 'LUM to bit conversion failed', { 
      error: error instanceof Error ? error.message : 'Unknown error',
      op: 'conversion_error'
    });
    res.status(500).json({ error: 'Conversion failed' });
  }
});

/**
 * @swagger
 * /api/execute/vorax-operation:
 *   post:
 *     tags: [Operations]
 *     summary: Execute a VORAX operation on LUM groups
 *     description: Performs fusion, split, cycle, or flow operations on LUM groups
 *     requestBody:
 *       required: true
 *       content:
 *         application/json:
 *           schema:
 *             $ref: '#/components/schemas/VoraxOperation'
 *     responses:
 *       200:
 *         description: Operation executed successfully
 *         content:
 *           application/json:
 *             schema:
 *               type: object
 *               properties:
 *                 result:
 *                   oneOf:
 *                     - $ref: '#/components/schemas/LUMGroup'
 *                     - type: array
 *                       items:
 *                         $ref: '#/components/schemas/LUMGroup'
 *                 operation_stats:
 *                   type: object
 *       400:
 *         description: Validation error
 */
router.post('/execute/vorax-operation', apiLimiter, validateVoraxOperation, handleValidationErrors, (req, res) => {
  const { type, groups, parameters = {} } = req.body;

  try {
    let result;
    const totalInputLums = groups.reduce((sum: number, group: any) => sum + group.lums.length, 0);

    switch (type) {
      case 'fusion':
        if (groups.length < 2) {
          return res.status(400).json({ error: 'Fusion requires at least 2 groups' });
        }
        result = {
          id: `fusion-${Date.now()}`,
          lums: groups.flatMap((group: any) => group.lums.map((lum: any, index: number) => ({
            ...lum,
            lum_id: lum.lum_id || `L-${logger.runId}-fused-${index.toString().padStart(6, '0')}`
          }))),
          groupType: 'cluster',
          metadata: { 
            operation: 'fusion', 
            timestamp: new Date().toISOString(),
            source_groups: groups.length
          }
        };
        break;

      case 'split':
        const zones = parameters.zones || 2;
        const sourceGroup = groups[0];
        const lumsPerZone = Math.floor(sourceGroup.lums.length / zones);
        const remainder = sourceGroup.lums.length % zones;

        result = [];
        let currentIndex = 0;

        for (let i = 0; i < zones; i++) {
          const zoneSize = lumsPerZone + (i < remainder ? 1 : 0);
          const zoneLums = sourceGroup.lums.slice(currentIndex, currentIndex + zoneSize)
            .map((lum: any, index: number) => ({
              ...lum,
              lum_id: lum.lum_id || `L-${logger.runId}-zone${i}-${index.toString().padStart(6, '0')}`
            }));

          result.push({
            id: `split-zone-${i}-${Date.now()}`,
            lums: zoneLums,
            groupType: 'linear',
            zone: `zone_${i}`,
            metadata: { 
              operation: 'split', 
              zone_index: i,
              zone_size: zoneSize
            }
          });
          currentIndex += zoneSize;
        }
        break;

      case 'cycle':
        const modulo = parameters.modulo || 3;
        const sourceGroup2 = groups[0];
        const cycledCount = sourceGroup2.lums.length % modulo;

        result = {
          id: `cycle-${Date.now()}`,
          lums: sourceGroup2.lums.slice(0, cycledCount).map((lum: any, index: number) => ({
            ...lum,
            lum_id: lum.lum_id || `L-${logger.runId}-cycle-${index.toString().padStart(6, '0')}`
          })),
          groupType: 'node',
          metadata: { 
            operation: 'cycle', 
            modulo,
            original_count: sourceGroup2.lums.length,
            cycled_count: cycledCount
          }
        };
        break;

      case 'flow':
        const targetZone = parameters.targetZone || 'default';
        result = {
          ...groups[0],
          id: `flow-${Date.now()}`,
          lums: groups[0].lums.map((lum: any, index: number) => ({
            ...lum,
            lum_id: lum.lum_id || `L-${logger.runId}-flow-${index.toString().padStart(6, '0')}`
          })),
          connections: [targetZone],
          metadata: { 
            operation: 'flow', 
            target_zone: targetZone,
            timestamp: new Date().toISOString()
          }
        };
        break;

      default:
        return res.status(400).json({ error: `Unknown operation: ${type}` });
    }

    // Vérification conservation pour toutes les opérations sauf cycle
    const resultLums = Array.isArray(result) 
      ? result.reduce((sum, group) => sum + group.lums.length, 0)
      : result.lums.length;

    const conservationValid = type === 'cycle' || resultLums === totalInputLums;

    // Log opération avec validation conservation complète
    logger.logLumOperation(
      type,
      'main',
      Array.isArray(result) ? result.flatMap(g => g.lums.map(l => l.lum_id || 'unknown')) : result.lums.map((l: any) => l.lum_id || 'unknown'),
      totalInputLums,
      resultLums,
      { 
        conservation_valid: conservationValid,
        parameters,
        operation_type: type,
        client_ip: req.ip
      }
    );

    res.json({
      result,
      operation_stats: {
        type,
        input_groups: groups.length,
        input_lums: totalInputLums,
        output_lums: resultLums,
        conservation_valid: conservationValid,
        parameters,
        run_id: logger.runId
      }
    });

  } catch (error) {
    logger.log('error', 'VORAX operation failed', { 
      error: error instanceof Error ? error.message : 'Unknown error',
      operation: type,
      groups_count: groups.length,
      op: 'vorax_operation_error'
    });
    res.status(500).json({ error: 'Operation failed' });
  }
});

/**
 * @swagger
 * /api/logs/save:
 *   post:
 *     tags: [Logging]
 *     summary: Save current session logs to JSONL file
 *     description: Persists the current session's logs to a JSONL file for analysis
 *     responses:
 *       200:
 *         description: Logs saved successfully
 *         content:
 *           application/json:
 *             schema:
 *               type: object
 *               properties:
 *                 message:
 *                   type: string
 *                 filepath:
 *                   type: string
 *                 log_count:
 *                   type: integer
 */
router.post('/logs/save', apiLimiter, (req, res) => {
  try {
    const filepath = logger.saveToJSONL();
    const stats = logger.getStats();

    res.json({
      message: 'Logs saved successfully',
      filepath,
      log_count: stats.total_logs,
      run_id: stats.run_id,
      conservation_rate: stats.conservation_stats.conservation_rate
    });

  } catch (error) {
    logger.log('error', 'Failed to save logs', { 
      error: error instanceof Error ? error.message : 'Unknown error',
      op: 'logs_save_error'
    });
    res.status(500).json({ error: 'Failed to save logs' });
  }
});

/**
 * @swagger
 * /api/logs/stats:
 *   get:
 *     tags: [Logging]
 *     summary: Get current session logging statistics
 *     description: Returns detailed statistics about the current logging session
 *     responses:
 *       200:
 *         description: Statistics retrieved successfully
 *         content:
 *           application/json:
 *             schema:
 *               $ref: '#/components/schemas/LogStats'
 */
router.get('/logs/stats', (req, res) => {
  try {
    const stats = logger.getStats();
    res.json(stats);
  } catch (error) {
    logger.log('error', 'Failed to get log stats', { 
      error: error instanceof Error ? error.message : 'Unknown error',
      op: 'logs_stats_error'
    });
    res.status(500).json({ error: 'Failed to get statistics' });
  }
});

export function registerRoutes(app: express.Application) {
  // Documentation API avec options personnalisées
  app.use('/api-docs', swaggerUi.serve, swaggerUi.setup(swaggerSpec, swaggerOptions));

  // Routes API avec sécurité renforcée
  app.use('/api', router);
}

export default router;