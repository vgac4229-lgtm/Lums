import fs from 'fs';
import path from 'path';

interface LUMTraceEntry {
  ts_ns: number;
  tick: number;
  tx_id: string;
  op: string;
  zone?: string;
  lum_id?: string;
  prev_count?: number;
  added?: number;
  removed?: number;
  post_count?: number;
  conservation_check?: boolean;
  energy_cost?: number;
}

interface VoraxTickEntry {
  tick: number;
  instruction: any;
  zones: number[];
  memory: number[];
  energy: number;
}

class LUMSLogger {
  private logFile: string;
  private recentLogs: LUMTraceEntry[] = [];
  private maxRecentLogs = 1000;

  constructor() {
    const logsDir = path.join(process.cwd(), 'logs');
    if (!fs.existsSync(logsDir)) {
      fs.mkdirSync(logsDir, { recursive: true });
    }

    const timestamp = new Date().toISOString().replace(/[:.]/g, '-');
    this.logFile = path.join(logsDir, `lums-${timestamp}.jsonl`);
  }

  private generateTxId(): string {
    return Math.random().toString(36).substr(2, 9);
  }

  private getNanoTimestamp(): number {
    const hrTime = process.hrtime();
    return hrTime[0] * 1e9 + hrTime[1];
  }

  logLUMOperation(operation: string, zone: string, lumId: string, details: Partial<LUMTraceEntry> = {}): void {
    const entry: LUMTraceEntry = {
      ts_ns: this.getNanoTimestamp(),
      tick: details.tick || 0,
      tx_id: this.generateTxId(),
      op: operation,
      zone,
      lum_id: lumId,
      ...details
    };

    // Write to JSONL file
    const jsonLine = JSON.stringify(entry) + '\n';
    fs.appendFileSync(this.logFile, jsonLine);

    // Keep in memory for recent access
    this.recentLogs.push(entry);
    if (this.recentLogs.length > this.maxRecentLogs) {
      this.recentLogs.shift();
    }
  }

  logConservationCheck(before: number, after: number, operation: string, zone: string): void {
    const isValid = before === after;
    this.logLUMOperation('conservation_check', zone, 'system', {
      prev_count: before,
      post_count: after,
      conservation_check: isValid,
      op: operation
    });

    if (!isValid) {
      this.error(`Conservation violation: ${operation} in ${zone}. Before: ${before}, After: ${after}`);
    }
  }

  logVoraxTick(tickData: VoraxTickEntry): void {
    this.logLUMOperation('vm_tick', 'system', 'vm', {
      tick: tickData.tick,
      energy_cost: tickData.energy,
      op: `opcode_0x${tickData.instruction.opcode.toString(16)}`
    });
  }

  getRecentLogs(limit: number = 100): LUMTraceEntry[] {
    return this.recentLogs.slice(-limit);
  }

  exportLogsToCSV(): string {
    const headers = ['timestamp', 'tick', 'tx_id', 'operation', 'zone', 'lum_id', 'prev_count', 'post_count', 'conservation_valid'];
    const csvData = this.recentLogs.map(log => [
      new Date(log.ts_ns / 1e6).toISOString(),
      log.tick,
      log.tx_id,
      log.op,
      log.zone || '',
      log.lum_id || '',
      log.prev_count || '',
      log.post_count || '',
      log.conservation_check !== undefined ? log.conservation_check : ''
    ]);

    return [headers, ...csvData].map(row => row.join(',')).join('\n');
  }

  info(message: string, ...args: any[]): void {
    console.log(`[INFO] ${message}`, ...args);
    this.logLUMOperation('info', 'system', 'logger', { op: message });
  }

  warn(message: string, ...args: any[]): void {
    console.warn(`[WARN] ${message}`, ...args);
    this.logLUMOperation('warn', 'system', 'logger', { op: message });
  }

  error(message: string, ...args: any[]): void {
    console.error(`[ERROR] ${message}`, ...args);
    this.logLUMOperation('error', 'system', 'logger', { op: message });
  }
}

export const logger = new LUMSLogger();