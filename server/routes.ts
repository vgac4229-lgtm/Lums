import express from 'express';
import { logger } from './logger';
import { swaggerUi, swaggerSpec } from './api-docs';

const router = express.Router();

export function registerRoutes(app: express.Application) {
  // Documentation API
  app.use('/api-docs', swaggerUi.serve, swaggerUi.setup(swaggerSpec));

  // Routes API
  app.use('/api', router);
}

export default router;