import { z } from "zod";

// Core LUM and Zone schemas
export const lumSchema = z.object({
  id: z.string(),
  presence: z.number().min(0).max(1),
  structureType: z.enum(['linear', 'group', 'node', 'cycle']),
  position: z.object({
    x: z.number(),
    y: z.number(),
  }),
});

export const zoneSchema = z.object({
  id: z.string(),
  name: z.string(),
  lumCount: z.number().min(0),
  bounds: z.object({
    x: z.number(),
    y: z.number(),
    width: z.number(),
    height: z.number(),
  }),
});

export const memorySlotSchema = z.object({
  id: z.string(),
  name: z.string(),
  lumCount: z.number().min(0),
  timestamp: z.number(),
});

export const logEntrySchema = z.object({
  level: z.enum(['info', 'success', 'warning', 'error']),
  message: z.string(),
  timestamp: z.string(),
  data: z.record(z.any()).optional(),
});

export const executionStepSchema = z.object({
  tick: z.number(),
  operation: z.string(),
  status: z.enum(['pending', 'executing', 'completed', 'failed']),
  params: z.record(z.any()).optional(),
});

export const executionMetricsSchema = z.object({
  executionTime: z.number(),
  memoryUsage: z.number(),
  lumOperations: z.number(),
  ticksExecuted: z.number(),
  totalTicks: z.number(),
  violations: z.number(),
  efficiency: z.number(),
  conservationValid: z.boolean(),
  raceConditions: z.number(),
});

export const voraxEngineStateSchema = z.object({
  zones: z.array(zoneSchema),
  memory: z.array(memorySlotSchema),
  logs: z.array(logEntrySchema),
  executionSteps: z.array(executionStepSchema),
  metrics: executionMetricsSchema,
  currentTick: z.number(),
  totalTicks: z.number(),
  isExecuting: z.boolean(),
});

export const executeRequestSchema = z.object({
  code: z.string(),
  statements: z.array(z.record(z.any())),
});

export const operationRequestSchema = z.object({
  operation: z.string(),
  sourceZone: z.string().optional(),
  targetZone: z.string().optional(),
  parameters: z.record(z.any()).optional(),
});

// Types
export type LUM = z.infer<typeof lumSchema>;
export type Zone = z.infer<typeof zoneSchema>;
export type MemorySlot = z.infer<typeof memorySlotSchema>;
export type LogEntry = z.infer<typeof logEntrySchema>;
export type ExecutionStep = z.infer<typeof executionStepSchema>;
export type ExecutionMetrics = z.infer<typeof executionMetricsSchema>;
export type VoraxEngineState = z.infer<typeof voraxEngineStateSchema>;
export type ExecuteRequest = z.infer<typeof executeRequestSchema>;
export type OperationRequest = z.infer<typeof operationRequestSchema>;

// User schemas for authentication (keeping existing structure)
export const insertUserSchema = z.object({
  username: z.string().min(1),
  password: z.string().min(6),
});

export type InsertUser = z.infer<typeof insertUserSchema>;

export interface User {
  id: string;
  username: string;
  password: string;
}
