import { LogEntry } from "@shared/schema";

export class VoraxLogger {
  private logs: LogEntry[] = [];
  private runId: string;

  constructor() {
    this.runId = `run-${Date.now()}`;
  }

  log(level: LogEntry['level'], message: string, data?: Record<string, any>): void {
    const logEntry: LogEntry = {
      level,
      message,
      timestamp: new Date().toISOString(),
      data: {
        ...data,
        runId: this.runId,
      },
    };

    this.logs.push(logEntry);
    console.log(`[${level.toUpperCase()}] ${message}`, data || '');
  }

  info(message: string, data?: Record<string, any>): void {
    this.log('info', message, data);
  }

  success(message: string, data?: Record<string, any>): void {
    this.log('success', message, data);
  }

  warning(message: string, data?: Record<string, any>): void {
    this.log('warning', message, data);
  }

  error(message: string, data?: Record<string, any>): void {
    this.log('error', message, data);
  }

  logLumOperation(
    operation: string,
    zone: string,
    lumIds: string[],
    beforeCount: number,
    afterCount: number,
    metadata?: Record<string, any>
  ): void {
    const conserved = beforeCount === afterCount || operation === 'cycle';
    
    this.log(conserved ? 'success' : 'error', `LUM operation: ${operation}`, {
      operation,
      zone,
      lumIds,
      beforeCount,
      afterCount,
      conserved,
      ...metadata,
    });
  }

  getLogs(): LogEntry[] {
    return [...this.logs];
  }

  clearLogs(): void {
    this.logs = [];
  }

  getRunId(): string {
    return this.runId;
  }

  getLogStats() {
    return {
      totalLogs: this.logs.length,
      runId: this.runId,
      levels: {
        info: this.logs.filter(l => l.level === 'info').length,
        success: this.logs.filter(l => l.level === 'success').length,
        warning: this.logs.filter(l => l.level === 'warning').length,
        error: this.logs.filter(l => l.level === 'error').length,
      },
    };
  }
}

export const logger = new VoraxLogger();
