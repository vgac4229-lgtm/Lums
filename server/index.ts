import express, { type Request, Response, NextFunction } from "express";
import cors from "cors";
import rateLimit from 'express-rate-limit';
import { logger } from './logger';
import { registerRoutes } from './routes';
import { setupVite, serveStatic, log } from "./vite";


const app = express();
const PORT = process.env.PORT || 5000;

// Rate limiting pour sécurité
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

// Middleware
app.use(cors());
app.use(express.json({ limit: '10mb' }));
app.use('/api', apiLimiter);

app.use((req, res, next) => {
  const start = Date.now();
  const path = req.path;
  let capturedJsonResponse: Record<string, any> | undefined = undefined;

  const originalResJson = res.json;
  res.json = function (bodyJson, ...args) {
    capturedJsonResponse = bodyJson;
    return originalResJson.apply(res, [bodyJson, ...args]);
  };

  res.on("finish", () => {
    const duration = Date.now() - start;
    if (path.startsWith("/api")) {
      let logLine = `${req.method} ${path} ${res.statusCode} in ${duration}ms`;
      if (capturedJsonResponse) {
        logLine += ` :: ${JSON.stringify(capturedJsonResponse)}`;
      }

      if (logLine.length > 80) {
        logLine = logLine.slice(0, 79) + "…";
      }

      log(logLine);
    }
  });

  next();
});

// Health check
app.get('/health', (req, res) => {
  res.json({ 
    status: 'healthy', 
    timestamp: new Date().toISOString(),
    version: '2.0.0',
    service: 'LUMS/VORAX API'
  });
});

// Routes
registerRoutes(app);

// Error handling
app.use((err: any, req: Request, res: Response, next: NextFunction) => {
  const status = err.status || err.statusCode || 500;
  const message = err.message || "Internal Server Error";

  logger.log('error', 'Unhandled error', { 
    error: err.message, 
    stack: err.stack,
    url: req.url,
    method: req.method,
    status: status
  });
  res.status(status).json({ message });
  // Original code had 'throw err;' here, which is generally not recommended in Express error handlers.
  // Instead, we've logged the error and sent a response. If further propagation is needed, it should be handled carefully.
});

// 404 handler
app.use((req, res) => {
  res.status(404).json({ 
    error: 'Endpoint not found',
    available_endpoints: ['/api-docs', '/api/*', '/health']
  });
});


// Start server
const server = app.listen(PORT, '0.0.0.0', () => {
  log(`serving on port ${PORT}`);
  logger.log('info', `Server starting on port ${PORT}`);
  logger.log('info', 'LUMS/VORAX API documentation available at /api-docs');
  logger.log('info', 'API endpoints registered at /api/*');
  logger.log('info', 'Rate limiting: 100 requests/15min per IP');
  logger.log('info', 'Ready to accept connections');
});

// Graceful shutdown
process.on('SIGTERM', () => {
  logger.log('info', 'SIGTERM received, shutting down gracefully');
  server.close(() => {
    logger.log('info', 'Server closed');
    process.exit(0);
  });
});

process.on('SIGINT', () => {
  logger.log('info', 'SIGINT received, shutting down gracefully');
  server.close(() => {
    logger.log('info', 'Server closed');
    process.exit(0);
  });
});
})();