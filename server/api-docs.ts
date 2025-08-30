import swaggerJsdoc from 'swagger-jsdoc';
import swaggerUi from 'swagger-ui-express';

const options = {
  definition: {
    openapi: '3.0.0',
    info: {
      title: 'LUMS/VORAX API',
      version: '1.0.0',
      description: 'API for LUMS (Lumaire) post-digital computing system with VORAX operations',
      contact: {
        name: 'LUMS/VORAX System',
        url: 'https://replit.com/@username/lums-vorax'
      }
    },
    servers: [
      {
        url: 'https://{project-name}.{username}.repl.co',
        description: 'Replit Production Server'
      },
      {
        url: 'http://localhost:5000',
        description: 'Development Server'
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
              minimum: 0,
              maximum: 1,
              description: 'LUM presence (0 or 1)'
            },
            structureType: {
              type: 'string',
              enum: ['linear', 'group', 'node', 'cycle'],
              description: 'LUM structure type'
            },
            position: {
              type: 'object',
              properties: {
                x: { type: 'number' },
                y: { type: 'number' }
              }
            },
            lum_id: {
              type: 'string',
              description: 'Unique LUM identifier for tracing'
            }
          }
        },
        LUMGroup: {
          type: 'object',
          required: ['lums', 'groupType'],
          properties: {
            lums: {
              type: 'array',
              items: { $ref: '#/components/schemas/LUM' }
            },
            groupType: {
              type: 'string',
              enum: ['linear', 'cluster', 'node', 'memory']
            },
            id: { type: 'string' }
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
              description: 'Input LUM groups'
            },
            parameters: {
              type: 'object',
              properties: {
                zones: { type: 'integer', minimum: 2 },
                modulo: { type: 'integer', minimum: 1 },
                targetZone: { type: 'string' }
              }
            }
          }
        },
        ApiError: {
          type: 'object',
          properties: {
            error: { type: 'string' },
            details: { type: 'string' },
            code: { type: 'integer' }
          }
        },
        LogStats: {
          type: 'object',
          properties: {
            total_logs: { type: 'integer' },
            run_id: { type: 'string' },
            current_tick: { type: 'integer' },
            levels: { type: 'object' },
            operations: { type: 'object' },
            duration_ms: { type: 'number' }
          }
        }
      }
    }
  },
  apis: ['./server/routes.ts']
};

export const swaggerSpec = swaggerJsdoc(options);
export { swaggerUi };