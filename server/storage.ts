import { type LumSession, type InsertLumSession, type LumOperation, type InsertLumOperation } from "@shared/schema";
import { randomUUID } from "crypto";

export interface IStorage {
  // LUM Sessions
  getLumSession(id: string): Promise<LumSession | undefined>;
  createLumSession(session: InsertLumSession): Promise<LumSession>;
  updateLumSession(id: string, updates: Partial<InsertLumSession>): Promise<LumSession | undefined>;
  deleteLumSession(id: string): Promise<boolean>;
  getAllLumSessions(): Promise<LumSession[]>;

  // LUM Operations
  getLumOperation(id: string): Promise<LumOperation | undefined>;
  createLumOperation(operation: InsertLumOperation): Promise<LumOperation>;
  getOperationsBySession(sessionId: string): Promise<LumOperation[]>;
  deleteOperation(id: string): Promise<boolean>;
}

export class MemStorage implements IStorage {
  private lumSessions: Map<string, LumSession>;
  private lumOperations: Map<string, LumOperation>;

  constructor() {
    this.lumSessions = new Map();
    this.lumOperations = new Map();
  }

  async getLumSession(id: string): Promise<LumSession | undefined> {
    return this.lumSessions.get(id);
  }

  async createLumSession(insertSession: InsertLumSession): Promise<LumSession> {
    const id = randomUUID();
    const session: LumSession = {
      ...insertSession,
      id,
      createdAt: new Date(),
    };
    this.lumSessions.set(id, session);
    return session;
  }

  async updateLumSession(id: string, updates: Partial<InsertLumSession>): Promise<LumSession | undefined> {
    const existing = this.lumSessions.get(id);
    if (!existing) return undefined;

    const updated: LumSession = {
      ...existing,
      ...updates,
    };
    this.lumSessions.set(id, updated);
    return updated;
  }

  async deleteLumSession(id: string): Promise<boolean> {
    return this.lumSessions.delete(id);
  }

  async getAllLumSessions(): Promise<LumSession[]> {
    return Array.from(this.lumSessions.values());
  }

  async getLumOperation(id: string): Promise<LumOperation | undefined> {
    return this.lumOperations.get(id);
  }

  async createLumOperation(insertOperation: InsertLumOperation): Promise<LumOperation> {
    const id = randomUUID();
    const operation: LumOperation = {
      ...insertOperation,
      id,
      timestamp: new Date(),
    };
    this.lumOperations.set(id, operation);
    return operation;
  }

  async getOperationsBySession(sessionId: string): Promise<LumOperation[]> {
    return Array.from(this.lumOperations.values())
      .filter(op => op.sessionId === sessionId);
  }

  async deleteOperation(id: string): Promise<boolean> {
    return this.lumOperations.delete(id);
  }
}

export const storage = new MemStorage();
