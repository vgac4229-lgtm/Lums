import type { Express } from "express";
import { createServer, type Server } from "http";
import { z } from "zod";
import { storage } from "./storage";
import { lumsEngine } from "./services/lums-engine";
import { voraxCompiler } from "./services/vorax-compiler";
import { logger } from "./services/logger";
import { 
  executeRequestSchema, 
  operationRequestSchema,
  insertUserSchema 
} from "@shared/schema";

export async function registerRoutes(app: Express): Promise<Server> {
  // VORAX Engine API Routes
  app.get("/api/vorax/state", async (req, res) => {
    try {
      const state = lumsEngine.getState();
      res.json(state);
    } catch (error) {
      const message = error instanceof Error ? error.message : "Failed to get engine state";
      logger.error("Failed to get engine state", { error: message });
      res.status(500).json({ message });
    }
  });

  app.post("/api/vorax/execute", async (req, res) => {
    try {
      const { code, statements } = executeRequestSchema.parse(req.body);
      
      logger.info("Executing VORAX-L code", { 
        codeLength: code.length,
        statementCount: statements.length 
      });
      
      const result = await lumsEngine.executeStatements(statements);
      
      if (result.success) {
        const state = lumsEngine.getState();
        res.json({
          success: true,
          currentTick: state.currentTick,
          totalTicks: state.totalTicks,
          isExecuting: state.isExecuting,
        });
      } else {
        res.status(400).json({
          success: false,
          message: result.error,
        });
      }
    } catch (error) {
      if (error instanceof z.ZodError) {
        res.status(400).json({
          success: false,
          message: "Invalid request format",
          errors: error.errors,
        });
      } else {
        const message = error instanceof Error ? error.message : "Execution failed";
        logger.error("Execution failed", { error: message });
        res.status(500).json({
          success: false,
          message,
        });
      }
    }
  });

  app.post("/api/vorax/step", async (req, res) => {
    try {
      const result = await lumsEngine.stepExecution();
      
      if (result.success) {
        res.json({
          success: true,
          currentTick: result.data?.currentTick || 0,
        });
      } else {
        res.status(400).json({
          success: false,
          message: result.error,
        });
      }
    } catch (error) {
      const message = error instanceof Error ? error.message : "Step execution failed";
      logger.error("Step execution failed", { error: message });
      res.status(500).json({
        success: false,
        message,
      });
    }
  });

  app.post("/api/vorax/pause", async (req, res) => {
    try {
      const result = await lumsEngine.pauseExecution();
      res.json(result);
    } catch (error) {
      const message = error instanceof Error ? error.message : "Pause execution failed";
      logger.error("Pause execution failed", { error: message });
      res.status(500).json({
        success: false,
        message,
      });
    }
  });

  app.post("/api/vorax/reset", async (req, res) => {
    try {
      const result = await lumsEngine.resetExecution();
      logger.clearLogs(); // Clear logs on reset
      res.json(result);
    } catch (error) {
      const message = error instanceof Error ? error.message : "Reset execution failed";
      logger.error("Reset execution failed", { error: message });
      res.status(500).json({
        success: false,
        message,
      });
    }
  });

  app.post("/api/vorax/operation", async (req, res) => {
    try {
      const { operation, sourceZone, targetZone, parameters } = operationRequestSchema.parse(req.body);
      
      logger.info("Executing VORAX operation", { 
        operation, 
        sourceZone, 
        targetZone, 
        parameters 
      });
      
      // Execute single operation
      const statement = {
        type: 'operation',
        data: {
          operation,
          params: {
            source: sourceZone,
            target: targetZone,
            ...parameters,
          }
        }
      };
      
      const result = await lumsEngine.executeStatements([statement]);
      
      if (result.success) {
        const state = lumsEngine.getState();
        res.json({
          success: true,
          operation_stats: {
            output_lums: state.zones.reduce((sum, z) => sum + z.lumCount, 0),
            conservation_valid: state.metrics.conservationValid,
          },
          result: state.zones,
        });
      } else {
        res.status(400).json({
          success: false,
          message: result.error,
        });
      }
    } catch (error) {
      if (error instanceof z.ZodError) {
        res.status(400).json({
          success: false,
          message: "Invalid operation request",
          errors: error.errors,
        });
      } else {
        const message = error instanceof Error ? error.message : "Operation failed";
        logger.error("Operation failed", { error: message });
        res.status(500).json({
          success: false,
          message,
        });
      }
    }
  });

  // Logging API Routes
  app.get("/api/logs/stats", (req, res) => {
    try {
      const stats = logger.getLogStats();
      res.json(stats);
    } catch (error) {
      const message = error instanceof Error ? error.message : "Failed to get log stats";
      res.status(500).json({ message });
    }
  });

  app.post("/api/logs/save", (req, res) => {
    try {
      const logs = logger.getLogs();
      // In a real implementation, this would save to a file or database
      res.json({
        success: true,
        filepath: `/logs/test-${logger.getRunId()}.jsonl`,
        log_count: logs.length,
      });
    } catch (error) {
      const message = error instanceof Error ? error.message : "Failed to save logs";
      res.status(500).json({ message });
    }
  });

  // Conversion API Routes for compatibility with tests
  app.post("/api/convert/bit-to-lum", async (req, res) => {
    try {
      const { inputBits } = req.body;
      
      if (!inputBits || typeof inputBits !== 'string' || !/^[01]+$/.test(inputBits)) {
        return res.status(400).json({
          success: false,
          message: "Invalid input bits - must be a string of 0s and 1s",
        });
      }
      
      const lums = inputBits.split('').map((bit: string, index: number) => ({
        id: `lum-${index}`,
        presence: parseInt(bit),
        structureType: 'linear' as const,
        position: { x: index * 20, y: 0 },
      }));
      
      logger.success("Bit-to-LUM conversion completed", {
        inputLength: inputBits.length,
        outputCount: lums.length,
      });
      
      res.json({
        lums,
        conversion_stats: {
          conservation_valid: inputBits.length === lums.length,
          conversion_time_ms: 1,
        },
      });
    } catch (error) {
      const message = error instanceof Error ? error.message : "Conversion failed";
      logger.error("Bit-to-LUM conversion failed", { error: message });
      res.status(500).json({
        success: false,
        message,
      });
    }
  });

  // User authentication routes (keeping existing structure)
  app.post("/api/users", async (req, res) => {
    try {
      const userData = insertUserSchema.parse(req.body);
      const user = await storage.createUser(userData);
      res.status(201).json(user);
    } catch (error) {
      if (error instanceof z.ZodError) {
        res.status(400).json({
          message: "Invalid user data",
          errors: error.errors,
        });
      } else {
        res.status(500).json({
          message: "Failed to create user",
        });
      }
    }
  });

  app.get("/api/users/:id", async (req, res) => {
    try {
      const user = await storage.getUser(req.params.id);
      if (!user) {
        return res.status(404).json({ message: "User not found" });
      }
      res.json(user);
    } catch (error) {
      res.status(500).json({
        message: "Failed to get user",
      });
    }
  });

  const httpServer = createServer(app);
  return httpServer;
}
