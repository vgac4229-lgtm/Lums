import swaggerJsdoc from 'swagger-jsdoc';
import swaggerUi from 'swagger-ui-express';

const options = {
  definition: {
    openapi: '3.0.0',
    info: {
      title: 'LUMS/VORAX API',
      version: '3.0.0',
      description: `
# LUMS/VORAX API - Post-Correction V3.0

This API provides full access to the LUMS (Lumaire) system with VORAX operations.

## Features

- **Bit ↔ LUM Conversion**: Bidirectional transformation with conservation
- **VORAX Operations**: Fusion, split, cycle, flow with validation
- **JSONL Logging**: LUM-by-LUM traceability with nanosecond timestamps
- **Security**: Rate limiting, strict validation, sanitization
- **Conservation**: Automatic invariant verification

## Architecture

- **Frontend**: React + TypeScript + HTML5 Canvas
- **Backend**: Node.js + Express + TypeScript
- **C Backend**: Native code compiled with Makefile
- **Testing**: Triple testing framework with automated validation

## Authentication

Currently no authentication required (development).
Production: Implement JWT or API keys.
      `,
      contact: {
        name: 'LUMS/VORAX Support',
        url: 'https://replit.com/@workspace/lums-vorax'
      }
    },
    servers: [
      {
        url: '/api',
        description: 'Development API'
      }
    ],
    components: {
      schemas: {
        LUM: {
          type: 'object',
          required: ['presence', 'structureType', 'position'],
          properties: {
            presence: {
              type: 'integer',
              enum: [0, 1],
              description: 'LUM presence (0 = absent, 1 = present)'
            },
            structureType: {
              type: 'string',
              enum: ['linear', 'clustered', 'memory', 'node'],
              description: 'LUM spatial structure type'
            },
            position: {
              type: 'object',
              required: ['x', 'y'],
              properties: {
                x: { type: 'number', description: 'X position' },
                y: { type: 'number', description: 'Y position' }
              }
            },
            lum_id: {
              type: 'string',
              pattern: '^L-[a-z0-9-]+-[0-9]{6}$',
              description: 'Unique LUM identifier',
              example: 'L-run-1642697412-abc123-000001'
            },
            metadata: {
              type: 'object',
              description: 'Additional LUM metadata'
            }
          },
          example: {
            presence: 1,
            structureType: 'linear',
            position: { x: 20, y: 0 },
            lum_id: 'L-run-1642697412-abc123-000001',
            metadata: {
              bit_position: 1,
              source_bit: '1',
              conversion_timestamp: '2025-01-20T20:30:12.123Z'
            }
          }
        },
        LUMGroup: {
          type: 'object',
          required: ['id', 'lums', 'groupType'],
          properties: {
            id: {
              type: 'string',
              description: 'Unique group identifier'
            },
            lums: {
              type: 'array',
              items: { $ref: '#/components/schemas/LUM' },
              description: 'List of LUMs in the group'
            },
            groupType: {
              type: 'string',
              enum: ['linear', 'cluster', 'node'],
              description: 'Group type'
            },
            zone: {
              type: 'string',
              description: 'Spatial zone of the group'
            },
            connections: {
              type: 'array',
              items: { type: 'string' },
              description: 'Connections to other zones'
            },
            metadata: {
              type: 'object',
              description: 'Group metadata'
            }
          }
        },
        VoraxOperation: {
          type: 'object',
          required: ['type', 'groups'],
          properties: {
            type: {
              type: 'string',
              enum: ['fusion', 'split', 'cycle', 'flow'],
              description: 'VORAX operation type'
            },
            groups: {
              type: 'array',
              items: { $ref: '#/components/schemas/LUMGroup' },
              minItems: 1,
              description: 'Groups of LUMs to process'
            },
            parameters: {
              type: 'object',
              properties: {
                zones: {
                  type: 'integer',
                  minimum: 2,
                  description: 'Number of zones for split operation'
                },
                modulo: {
                  type: 'integer',
                  minimum: 1,
                  description: 'Modulo for cycle operation'
                },
                targetZone: {
                  type: 'string',
                  description: 'Target zone for flow operation'
                }
              }
            }
          },
          example: {
            type: 'fusion',
            groups: [
              {
                id: 'group1',
                lums: [
                  { presence: 1, structureType: 'linear', position: { x: 0, y: 0 } }
                ],
                groupType: 'linear'
              }
            ]
          }
        },
        LogStats: {
          type: 'object',
          properties: {
            run_id: {
              type: 'string',
              description: 'Unique session ID'
            },
            total_logs: {
              type: 'integer',
              description: 'Total number of logs'
            },
            current_tick: {
              type: 'integer',
              description: 'Current system tick'
            },
            duration_ms: {
              type: 'number',
              description: 'Session duration in ms'
            },
            levels: {
              type: 'object',
              description: 'Log level distribution'
            },
            operations: {
              type: 'object',
              description: 'Operation type distribution'
            },
            conservation_stats: {
              type: 'object',
              properties: {
                total_operations: { type: 'integer' },
                conserved_operations: { type: 'integer' },
                violated_operations: { type: 'integer' },
                conservation_rate: { type: 'number' }
              }
            }
          }
        },
        ApiError: {
          type: 'object',
          required: ['error'],
          properties: {
            error: {
              type: 'string',
              description: 'Error message'
            },
            details: {
              type: 'array',
              items: { type: 'object' },
              description: 'Validation error details'
            }
          }
        }
      },
      securitySchemes: {
        ApiKeyAuth: {
          type: 'apiKey',
          in: 'header',
          name: 'X-API-Key',
          description: 'API Key for authentication (development: not implemented)'
        }
      }
    },
    tags: [
      {
        name: 'Conversion',
        description: 'Conversion between bit and LUM representations'
      },
      {
        name: 'Operations',
        description: 'VORAX operations on LUM groups'
      },
      {
        name: 'Logging',
        description: 'JSONL log management and statistics'
      }
    ]
  },
  apis: ['./server/routes.ts'],
};

export const swaggerSpec = swaggerJsdoc(options);

export const swaggerOptions = {
  explorer: true,
  customCss: `
    .swagger-ui .topbar { display: none; }
    .swagger-ui .info h1 { color: #059669; }
    .swagger-ui .scheme-container { background: #f0fdf4; padding: 10px; }
  `,
  customSiteTitle: 'LUMS/VORAX API Documentation',
  customfavIcon: '/favicon.ico'
};

export { swaggerUi };