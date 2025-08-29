
import swaggerJsdoc from 'swagger-jsdoc';
import swaggerUi from 'swagger-ui-express';

const options = {
  definition: {
    openapi: '3.0.0',
    info: {
      title: 'LUMS/VORAX API',
      version: '1.0.0',
      description: `
# LUMS/VORAX API Documentation

Cette API fournit l'accès programmatique au système LUMS (Lumaire) et aux opérations VORAX.

## Concepts fondamentaux

### LUM (Lumaire)
Un LUM représente l'unité de base du calcul post-numérique :
- **Présence** : 0 (absence) ou 1 (présence)
- **Structure** : linear, group, node, cycle
- **Position** : coordonnées spatiales (x, y)

### Opérations VORAX
- **Fusion** (⧉) : Combine deux groupes de LUMs
- **Division** (⇅) : Répartit un groupe en zones
- **Cycle** (⟲) : Applique un modulo sur la taille du groupe
- **Flux** (→) : Déplace un groupe vers une zone cible

## Sécurité
- Rate limiting : 100 requêtes/15 minutes par IP
- Validation stricte des entrées
- Logs JSONL détaillés pour traçabilité
      `,
      contact: {
        name: 'LUMS/VORAX Support',
        email: 'support@lums-vorax.example'
      },
      license: {
        name: 'MIT',
        url: 'https://opensource.org/licenses/MIT'
      }
    },
    servers: [
      {
        url: 'http://localhost:5000',
        description: 'Development server'
      },
      {
        url: 'https://production.lums-vorax.example',
        description: 'Production server'
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
              description: 'État de présence du LUM (0 = absent, 1 = présent)'
            },
            structureType: {
              type: 'string',
              enum: ['linear', 'group', 'node', 'cycle'],
              description: 'Type de structure spatiale du LUM'
            },
            position: {
              type: 'object',
              required: ['x', 'y'],
              properties: {
                x: {
                  type: 'number',
                  description: 'Coordonnée X spatiale'
                },
                y: {
                  type: 'number',
                  description: 'Coordonnée Y spatiale'
                }
              }
            },
            lum_id: {
              type: 'string',
              pattern: '^L-[a-zA-Z0-9]+-[0-9]+$',
              description: 'Identifiant unique du LUM'
            },
            metadata: {
              type: 'object',
              description: 'Métadonnées additionnelles du LUM'
            }
          },
          example: {
            presence: 1,
            structureType: 'linear',
            position: { x: 20, y: 0 },
            lum_id: 'L-run123-000001',
            metadata: {}
          }
        },
        LUMGroup: {
          type: 'object',
          required: ['lums'],
          properties: {
            lums: {
              type: 'array',
              items: { $ref: '#/components/schemas/LUM' },
              description: 'Liste des LUMs dans le groupe'
            },
            zone: {
              type: 'string',
              description: 'Nom de la zone spatiale'
            },
            metadata: {
              type: 'object',
              description: 'Métadonnées du groupe'
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
              description: 'Type d\'opération VORAX'
            },
            groups: {
              type: 'array',
              items: { $ref: '#/components/schemas/LUMGroup' },
              description: 'Groupes de LUMs à traiter'
            },
            parameters: {
              type: 'object',
              properties: {
                zones: {
                  type: 'integer',
                  minimum: 2,
                  description: 'Nombre de zones pour l\'opération split'
                },
                modulus: {
                  type: 'integer',
                  minimum: 1,
                  description: 'Modulo pour l\'opération cycle'
                },
                targetZone: {
                  type: 'string',
                  description: 'Zone cible pour l\'opération flow'
                }
              }
            }
          }
        },
        ApiError: {
          type: 'object',
          properties: {
            error: {
              type: 'string',
              description: 'Message d\'erreur'
            },
            details: {
              type: 'array',
              items: {
                type: 'object',
                properties: {
                  field: { type: 'string' },
                  message: { type: 'string' }
                }
              },
              description: 'Détails des erreurs de validation'
            }
          }
        },
        LogStats: {
          type: 'object',
          properties: {
            total_logs: {
              type: 'integer',
              description: 'Nombre total de logs'
            },
            run_id: {
              type: 'string',
              description: 'Identifiant de la session en cours'
            },
            current_tick: {
              type: 'integer',
              description: 'Tick actuel du système'
            },
            levels: {
              type: 'object',
              description: 'Répartition des logs par niveau'
            },
            operations: {
              type: 'object',
              description: 'Répartition des logs par opération'
            },
            duration_ms: {
              type: 'number',
              description: 'Durée de la session en millisecondes'
            }
          }
        }
      },
      securitySchemes: {
        ApiKeyAuth: {
          type: 'apiKey',
          in: 'header',
          name: 'X-API-Key',
          description: 'Clé API pour l\'authentification (future implémentation)'
        }
      }
    },
    tags: [
      {
        name: 'Conversion',
        description: 'Conversion entre représentations bit et LUM'
      },
      {
        name: 'Operations',
        description: 'Opérations VORAX sur les groupes de LUMs'
      },
      {
        name: 'Logging',
        description: 'Gestion des logs et traçabilité'
      },
      {
        name: 'System',
        description: 'Endpoints système et monitoring'
      }
    ]
  },
  apis: ['./server/routes.ts'], // Chemins vers les fichiers contenant les annotations Swagger
};

export const swaggerSpec = swaggerJsdoc(options);
export { swaggerUi };

// Configuration Swagger UI avec thème personnalisé
export const swaggerOptions = {
  customCss: `
    .swagger-ui .topbar { display: none }
    .swagger-ui .info .title { color: #2563eb }
    .swagger-ui .info .description p { font-size: 14px; line-height: 1.6 }
  `,
  customSiteTitle: 'LUMS/VORAX API Documentation',
  swaggerOptions: {
    persistAuthorization: true,
    displayRequestDuration: true,
    filter: true,
    showExtensions: true,
    showCommonExtensions: true,
    defaultModelsExpandDepth: 2,
    defaultModelExpandDepth: 3
  }
};
