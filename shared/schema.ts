import { sql } from "drizzle-orm";
import { pgTable, text, varchar, integer, jsonb, timestamp } from "drizzle-orm/pg-core";
import { createInsertSchema } from "drizzle-zod";
import { z } from "zod";

export const lumSessions = pgTable("lum_sessions", {
  id: varchar("id").primaryKey().default(sql`gen_random_uuid()`),
  name: text("name").notNull(),
  inputBits: text("input_bits").notNull(),
  outputLums: jsonb("output_lums").notNull(),
  operations: jsonb("operations").notNull().default('[]'),
  voraxCode: text("vorax_code"),
  createdAt: timestamp("created_at").defaultNow(),
});

export const lumOperations = pgTable("lum_operations", {
  id: varchar("id").primaryKey().default(sql`gen_random_uuid()`),
  sessionId: varchar("session_id").references(() => lumSessions.id),
  operationType: text("operation_type").notNull(), // fusion, split, cycle, flow
  inputLums: jsonb("input_lums").notNull(),
  outputLums: jsonb("output_lums").notNull(),
  parameters: jsonb("parameters").notNull().default('{}'),
  timestamp: timestamp("timestamp").defaultNow(),
});

export const insertLumSessionSchema = createInsertSchema(lumSessions).pick({
  name: true,
  inputBits: true,
  outputLums: true,
  operations: true,
  voraxCode: true,
});

export const insertLumOperationSchema = createInsertSchema(lumOperations).pick({
  sessionId: true,
  operationType: true,
  inputLums: true,
  outputLums: true,
  parameters: true,
});

export type InsertLumSession = z.infer<typeof insertLumSessionSchema>;
export type LumSession = typeof lumSessions.$inferSelect;
export type InsertLumOperation = z.infer<typeof insertLumOperationSchema>;
export type LumOperation = typeof lumOperations.$inferSelect;

// Core LUMS types
export const lumSchema = z.object({
  presence: z.number().min(0).max(1),
  structureType: z.enum(['linear', 'group', 'node', 'cycle']),
  spatialData: z.record(z.any()).optional(),
  position: z.object({
    x: z.number(),
    y: z.number(),
  }).optional(),
});

export const lumGroupSchema = z.object({
  id: z.string(),
  lums: z.array(lumSchema),
  groupType: z.enum(['linear', 'cluster', 'node', 'memory']),
  connections: z.array(z.string()).optional(),
});

export const voraxOperationSchema = z.object({
  symbol: z.enum(['⧉', '⇅', '⟲', '→']),
  name: z.enum(['fusion', 'split', 'cycle', 'flow']),
  sourceGroups: z.array(z.string()),
  targetGroups: z.array(z.string()),
  parameters: z.record(z.any()).optional(),
});

export type LUM = z.infer<typeof lumSchema>;
export type LUMGroup = z.infer<typeof lumGroupSchema>;
export type VoraxOperation = z.infer<typeof voraxOperationSchema>;
