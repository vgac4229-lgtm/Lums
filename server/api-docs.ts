

import swaggerJsdoc from 'swagger-jsdoc';
import swaggerUi from 'swagger-ui-express';

const options = {
  definition: {
    openapi: '3.0.0',
    info: {
      title: 'LUMS/VORAX API',
      version: '2.0.0',
      description: `
# LUMS/VORAX API Documentation v2.0

## 🚀 Système post-numérique de calcul spatial

Cette API fournit l'accès programmatique au système LUMS (Lumaire) et aux opérations VORAX avec traçabilité complète et validation triple.

## 🔬 Concepts fondamentaux

### LUM (Lumaire)
Un LUM représente l'unité de base du calcul post-numérique :
- **Présence** : 0 (absence) ou 1 (présence) - pas de valeur numérique abstraite
- **Structure** : linear, group, node, cycle - influence spatiale
- **Position** : coordonnées spatiales (x, y) - localisation dans l'espace
- **ID unique** : Format \`L-{runId}-{index}\` pour traçabilité LUM-par-LUM

### Opérations VORAX (Conservation & Transformation)
- **Fusion** (⧉) : Combine deux groupes de LUMs avec conservation stricte
- **Division** (⇅) : Répartit un groupe en zones avec conservation équitable
- **Cycle** (⟲) : Applique un modulo sur la taille du groupe (pas de conservation)
- **Flux** (→) : Déplace un groupe vers une zone cible avec métadonnées

## 🛡️ Sécurité et Validation
- **Rate limiting** : 100 requêtes/15 minutes par IP
- **Validation stricte** : Regex et types pour toutes les entrées
- **Sanitization** : Protection contre injections et DoS
- **Logs JSONL** : Traçabilité complète avec timestamps nanoseconde

## 📊 Monitoring et Observabilité
- **Conservation tracking** : Validation automatique des lois de conservation
- **Performance metrics** : Temps de réponse et statistiques d'utilisation
- **Error tracking** : Logs détaillés pour debugging
- **Session management** : Isolation des sessions avec IDs uniques

## 🔄 Tests et Qualité
- **Tests triples** : Chaque fonctionnalité testée 3 fois consécutives
- **Validation roundtrip** : Conversion bit→LUM→bit garantie
- **Invariants système** : Vérification continue des propriétés LUM
      `,
      contact: {
        name: 'LUMS/VORAX Support',
        email: 'support@lums-vorax.dev'
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
        url: 'https://production.lums-vorax.dev',
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
              description: 'État de présence du LUM (0 = absent, 1 = présent)',
              example: 1
            },
            structureType: {
              type: 'string',
              enum: ['linear', 'group', 'node', 'cycle'],
              description: 'Type de structure spatiale du LUM',
              example: 'linear'
            },
            position: {
              type: 'object',
              required: ['x', 'y'],
              properties: {
                x: {
                  type: 'number',
                  description: 'Coordonnée X spatiale',
                  example: 20
                },
                y: {
                  type: 'number',
                  description: 'Coordonnée Y spatiale',
                  example: 0
                }
              },
              description: 'Position spatiale du LUM'
            },
            lum_id: {
              type: 'string',
              pattern: '^L-[a-zA-Z0-9]+-[0-9]+$',
              description: 'Identifiant unique du LUM (format: L-{runId}-{index})',
              example: 'L-run-1642697412-abc123-000001'
            },
            metadata: {
              type: 'object',
              description: 'Métadonnées additionnelles du LUM',
              properties: {
                bit_position: {
                  type: 'integer',
                  description: 'Position du bit source lors de la conversion'
                },
                source_bit: {
                  type: 'string',
                  pattern: '^[01]$',
                  description: 'Bit source de la conversion'
                },
                conversion_timestamp: {
                  type: 'string',
                  format: 'date-time',
                  description: 'Timestamp de création du LUM'
                }
              }
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
              conversion_timestamp: '2025-01-20T10:30:00.000Z'
            }
          }
        },
        LUMGroup: {
          type: 'object',
          required: ['lums'],
          properties: {
            id: {
              type: 'string',
              description: 'Identifiant unique du groupe',
              example: 'group-1642697412-abc123def'
            },
            lums: {
              type: 'array',
              items: { $ref: '#/components/schemas/LUM' },
              description: 'Liste des LUMs dans le groupe',
              minItems: 1
            },
            groupType: {
              type: 'string',
              enum: ['linear', 'cluster', 'node', 'memory'],
              description: 'Type de groupement spatial',
              example: 'linear'
            },
            zone: {
              type: 'string',
              description: 'Nom de la zone spatiale',
              example: 'zone_A'
            },
            connections: {
              type: 'array',
              items: { type: 'string' },
              description: 'Zones connectées pour les opérations de flux'
            },
            metadata: {
              type: 'object',
              description: 'Métadonnées du groupe',
              properties: {
                operation: {
                  type: 'string',
                  description: 'Opération qui a créé ce groupe'
                },
                timestamp: {
                  type: 'string',
                  format: 'date-time',
                  description: 'Timestamp de création'
                },
                conservation_valid: {
                  type: 'boolean',
                  description: 'Validité de la conservation lors de la création'
                }
              }
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
              description: 'Type d\'opération VORAX',
              example: 'fusion'
            },
            groups: {
              type: 'array',
              items: { $ref: '#/components/schemas/LUMGroup' },
              description: 'Groupes de LUMs à traiter',
              minItems: 1
            },
            parameters: {
              type: 'object',
              description: 'Paramètres spécifiques à l\'opération',
              properties: {
                zones: {
                  type: 'integer',
                  minimum: 2,
                  maximum: 10,
                  description: 'Nombre de zones pour l\'opération split',
                  example: 2
                },
                modulus: {
                  type: 'integer',
                  minimum: 1,
                  maximum: 100,
                  description: 'Modulo pour l\'opération cycle',
                  example: 3
                },
                targetZone: {
                  type: 'string',
                  description: 'Zone cible pour l\'opération flow',
                  example: 'zone_target'
                }
              }
            }
          }
        },
        ConversionRequest: {
          type: 'object',
          required: ['inputBits'],
          properties: {
            inputBits: {
              type: 'string',
              pattern: '^[01]+$',
              minLength: 1,
              maxLength: 1000,
              description: 'Chaîne binaire à convertir en LUMs',
              example: '110100101'
            }
          }
        },
        ConversionResponse: {
          type: 'object',
          properties: {
            lums: {
              type: 'array',
              items: { $ref: '#/components/schemas/LUM' },
              description: 'LUMs résultant de la conversion'
            },
            conversion_stats: {
              type: 'object',
              properties: {
                input_length: {
                  type: 'integer',
                  description: 'Longueur de l\'entrée binaire'
                },
                output_count: {
                  type: 'integer',
                  description: 'Nombre de LUMs créés'
                },
                conversion_time_ms: {
                  type: 'number',
                  description: 'Temps de conversion en millisecondes'
                },
                run_id: {
                  type: 'string',
                  description: 'Identifiant de session'
                }
              }
            }
          }
        },
        OperationResponse: {
          type: 'object',
          properties: {
            result: {
              oneOf: [
                { $ref: '#/components/schemas/LUMGroup' },
                {
                  type: 'array',
                  items: { $ref: '#/components/schemas/LUMGroup' }
                }
              ],
              description: 'Résultat de l\'opération VORAX'
            },
            operation_stats: {
              type: 'object',
              properties: {
                type: {
                  type: 'string',
                  description: 'Type d\'opération exécutée'
                },
                input_groups: {
                  type: 'integer',
                  description: 'Nombre de groupes en entrée'
                },
                input_lums: {
                  type: 'integer',
                  description: 'Nombre total de LUMs en entrée'
                },
                output_lums: {
                  type: 'integer',
                  description: 'Nombre total de LUMs en sortie'
                },
                conservation_valid: {
                  type: 'boolean',
                  description: 'Respect de la loi de conservation'
                },
                parameters: {
                  type: 'object',
                  description: 'Paramètres utilisés pour l\'opération'
                }
              }
            }
          }
        },
        LogStats: {
          type: 'object',
          properties: {
            total_logs: {
              type: 'integer',
              description: 'Nombre total de logs dans la session'
            },
            run_id: {
              type: 'string',
              description: 'Identifiant de la session en cours'
            },
            current_tick: {
              type: 'integer',
              description: 'Tick actuel du système'
            },
            session_duration_ms: {
              type: 'number',
              description: 'Durée de la session en millisecondes'
            },
            levels: {
              type: 'object',
              description: 'Répartition des logs par niveau (debug, info, warn, error, success)',
              additionalProperties: {
                type: 'integer'
              }
            },
            operations: {
              type: 'object',
              description: 'Répartition des logs par opération',
              additionalProperties: {
                type: 'integer'
              }
            },
            conservation_stats: {
              type: 'object',
              properties: {
                total_operations: {
                  type: 'integer',
                  description: 'Nombre total d\'opérations avec tracking conservation'
                },
                conserved_operations: {
                  type: 'integer',
                  description: 'Nombre d\'opérations respectant la conservation'
                },
                violated_operations: {
                  type: 'integer',
                  description: 'Nombre d\'opérations violant la conservation'
                },
                conservation_rate: {
                  type: 'number',
                  minimum: 0,
                  maximum: 1,
                  description: 'Taux de respect de la conservation (0.0 à 1.0)'
                }
              }
            },
            performance: {
              type: 'object',
              properties: {
                logs_per_second: {
                  type: 'integer',
                  description: 'Nombre de logs générés par seconde'
                },
                avg_logs_per_tick: {
                  type: 'integer',
                  description: 'Nombre moyen de logs par tick'
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
              description: 'Message d\'erreur principal'
            },
            details: {
              type: 'array',
              items: {
                type: 'object',
                properties: {
                  field: { 
                    type: 'string',
                    description: 'Champ concerné par l\'erreur'
                  },
                  message: { 
                    type: 'string',
                    description: 'Message d\'erreur détaillé'
                  },
                  value: {
                    description: 'Valeur invalide reçue'
                  }
                }
              },
              description: 'Détails des erreurs de validation'
            },
            timestamp: {
              type: 'string',
              format: 'date-time',
              description: 'Timestamp de l\'erreur'
            },
            run_id: {
              type: 'string',
              description: 'Identifiant de session pour le debugging'
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
        description: 'Conversion entre représentations bit et LUM avec validation complète'
      },
      {
        name: 'Operations',
        description: 'Opérations VORAX sur les groupes de LUMs avec conservation tracking'
      },
      {
        name: 'Logging',
        description: 'Gestion des logs JSONL et traçabilité LUM-par-LUM'
      },
      {
        name: 'System',
        description: 'Endpoints système, monitoring et health checks'
      }
    ]
  },
  apis: ['./server/routes.ts'], // Chemins vers les fichiers contenant les annotations Swagger
};

export const swaggerSpec = swaggerJsdoc(options);
export { swaggerUi };

// Configuration Swagger UI avec thème personnalisé et fonctionnalités avancées
export const swaggerOptions = {
  customCss: `
    .swagger-ui .topbar { display: none }
    .swagger-ui .info .title { 
      color: #2563eb; 
      font-size: 2.5rem;
      font-weight: bold;
    }
    .swagger-ui .info .description p { 
      font-size: 14px; 
      line-height: 1.6;
      color: #374151;
    }
    .swagger-ui .info .description h1 {
      color: #1f2937;
      border-bottom: 2px solid #e5e7eb;
      padding-bottom: 0.5rem;
    }
    .swagger-ui .info .description h2 {
      color: #374151;
      margin-top: 1.5rem;
    }
    .swagger-ui .info .description h3 {
      color: #4b5563;
    }
    .swagger-ui .scheme-container {
      background: #f9fafb;
      border: 1px solid #e5e7eb;
      border-radius: 6px;
      padding: 1rem;
    }
    .swagger-ui .opblock.opblock-post {
      border-color: #059669;
      background: rgba(5, 150, 105, 0.1);
    }
    .swagger-ui .opblock.opblock-get {
      border-color: #2563eb;
      background: rgba(37, 99, 235, 0.1);
    }
    .swagger-ui .opblock-summary {
      font-weight: 600;
    }
    .swagger-ui .response-col_status {
      font-family: 'Monaco', 'Menlo', monospace;
    }
  `,
  customSiteTitle: 'LUMS/VORAX API Documentation v2.0',
  customfavIcon: '/favicon.ico',
  swaggerOptions: {
    persistAuthorization: true,
    displayRequestDuration: true,
    filter: true,
    showExtensions: true,
    showCommonExtensions: true,
    defaultModelsExpandDepth: 3,
    defaultModelExpandDepth: 3,
    docExpansion: 'list',
    operationsSorter: 'alpha',
    tagsSorter: 'alpha',
    tryItOutEnabled: true,
    requestInterceptor: (req: any) => {
      // Ajout automatique du run_id dans les headers pour traçabilité
      req.headers['X-Run-ID'] = `swagger-${Date.now()}`;
      return req;
    },
    responseInterceptor: (res: any) => {
      // Log des réponses pour debugging
      console.log('API Response:', res.status, res.url);
      return res;
    }
  }
};

