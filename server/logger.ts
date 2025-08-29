
import fs from 'fs';
import path from 'path';

interface LUMLogEntry {
  ts_ns: string;
  run_id: string;
  tick?: number;
  tx_id?: string;
  op: string;
  zone?: string;
  lum_id?: string;
  prev_count?: number;
  added?: number;
  removed?: number;
  post_count?: number;
  level: 'debug' | 'info' | 'warn' | 'error' | 'success';
  message: string;
  metadata?: Record<string, any>;
}

class LUMLogger {
  private logs: LUMLogEntry[] = [];
  private runId: string;
  private currentTick: number = 0;
  private logsDir: string;

  constructor() {
    this.runId = `run-${Date.now()}-${Math.random().toString(36).substr(2, 9)}`;
    this.logsDir = path.join(process.cwd(), 'logs');
    this.ensureLogsDirectory();
  }

  private ensureLogsDirectory(): void {
    if (!fs.existsSync(this.logsDir)) {
      fs.mkdirSync(this.logsDir, { recursive: true });
    }
  }

  private generateTxId(): string {
    return Math.random().toString(36).substr(2, 8);
  }

  incrementTick(): void {
    this.currentTick++;
  }

  log(
    level: LUMLogEntry['level'],
    message: string,
    metadata: Partial<Omit<LUMLogEntry, 'ts_ns' | 'run_id' | 'level' | 'message'>> = {}
  ): void {
    const entry: LUMLogEntry = {
      ts_ns: process.hrtime.bigint().toString(),
      run_id: this.runId,
      tick: this.currentTick,
      tx_id: metadata.tx_id || this.generateTxId(),
      op: metadata.op || 'unknown',
      level,
      message,
      ...metadata
    };

    this.logs.push(entry);

    // Log immédiat pour debug
    if (process.env.NODE_ENV === 'development') {
      console.log(`[${entry.level.toUpperCase()}] ${entry.message}`, 
        entry.metadata ? JSON.stringify(entry.metadata, null, 2) : '');
    }
  }

  // Logs spécialisés pour les opérations LUM
  logLumOperation(
    op: string,
    zone: string,
    lumIds: string[],
    prevCount: number,
    postCount: number,
    metadata: Record<string, any> = {}
  ): void {
    const txId = this.generateTxId();
    
    // Log principal de l'opération
    this.log('info', `LUM operation: ${op}`, {
      op,
      zone,
      tx_id: txId,
      prev_count: prevCount,
      post_count: postCount,
      added: postCount - prevCount,
      removed: prevCount - postCount,
      metadata
    });

    // Log individuel pour chaque LUM affecté
    lumIds.forEach((lumId, index) => {
      this.log('debug', `LUM ${op} individual`, {
        op: `${op}_individual`,
        zone,
        lum_id: lumId,
        tx_id: txId,
        lum_index: index,
        metadata
      });
    });
  }

  logConversion(inputBits: string, outputLums: any[], metadata: Record<string, any> = {}): void {
    const txId = this.generateTxId();
    
    this.log('info', 'Bit to LUM conversion', {
      op: 'conversion',
      tx_id: txId,
      prev_count: 0,
      post_count: outputLums.length,
      added: outputLums.length,
      metadata: {
        input_bits: inputBits,
        input_length: inputBits.length,
        output_length: outputLums.length,
        conversion_rate: outputLums.length / inputBits.length,
        ...metadata
      }
    });

    // Log pour chaque LUM créé
    outputLums.forEach((lum, index) => {
      this.log('debug', 'LUM created from bit', {
        op: 'bit_to_lum',
        tx_id: txId,
        lum_id: lum.lum_id || `L-${this.runId}-${index.toString().padStart(6, '0')}`,
        metadata: {
          bit_position: index,
          bit_value: inputBits[index],
          lum_presence: lum.presence,
          ...metadata
        }
      });
    });
  }

  logValidation(target: string, success: boolean, errors: string[] = [], metadata: Record<string, any> = {}): void {
    this.log(success ? 'success' : 'error', `Validation ${success ? 'passed' : 'failed'}: ${target}`, {
      op: 'validation',
      tx_id: this.generateTxId(),
      metadata: {
        target,
        success,
        error_count: errors.length,
        errors,
        ...metadata
      }
    });
  }

  saveToJSONL(): string {
    const timestamp = new Date().toISOString().replace(/[:.]/g, '-');
    const filename = `${this.runId}-${timestamp}.jsonl`;
    const filepath = path.join(this.logsDir, filename);

    const jsonlContent = this.logs.map(log => JSON.stringify(log)).join('\n');
    fs.writeFileSync(filepath, jsonlContent);

    this.log('info', `Logs saved to JSONL`, {
      op: 'save_logs',
      metadata: {
        filepath,
        log_count: this.logs.length,
        file_size: Buffer.byteLength(jsonlContent, 'utf8')
      }
    });

    return filepath;
  }

  getStats(): Record<string, any> {
    const stats = {
      total_logs: this.logs.length,
      run_id: this.runId,
      current_tick: this.currentTick,
      levels: {} as Record<string, number>,
      operations: {} as Record<string, number>,
      duration_ms: 0
    };

    // Calcul de la durée
    if (this.logs.length > 0) {
      const firstLog = BigInt(this.logs[0].ts_ns);
      const lastLog = BigInt(this.logs[this.logs.length - 1].ts_ns);
      stats.duration_ms = Number((lastLog - firstLog) / BigInt(1000000));
    }

    // Statistiques par niveau et opération
    this.logs.forEach(log => {
      stats.levels[log.level] = (stats.levels[log.level] || 0) + 1;
      stats.operations[log.op] = (stats.operations[log.op] || 0) + 1;
    });

    return stats;
  }

  flush(): void {
    this.logs = [];
    this.currentTick = 0;
  }
}

// Instance singleton
export const logger = new LUMLogger();

// Export du type pour utilisation externe
export type { LUMLogEntry };
