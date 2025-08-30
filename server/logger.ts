import fs from 'fs';
import path from 'path';

interface LogEntry {
  ts_ns: string;
  run_id: string;
  tick: number;
  tx_id: string;
  op: string;
  zone?: string;
  lum_id?: string;
  prev_count?: number;
  added?: number;
  post_count?: number;
  level: 'debug' | 'info' | 'success' | 'warn' | 'error';
  message: string;
  [key: string]: any;
}

class LUMLogger {
  public runId: string;
  public currentTick: number = 0;
  private logs: LogEntry[] = [];
  private startTime: bigint;

  constructor() {
    this.runId = `run-${Date.now()}-${Math.random().toString(36).substr(2, 6)}`;
    this.startTime = process.hrtime.bigint();

    // Ensure logs directory exists
    const logsDir = path.join(process.cwd(), 'logs');
    if (!fs.existsSync(logsDir)) {
      fs.mkdirSync(logsDir, { recursive: true });
    }

    this.log('info', 'Logger initialized', { 
      run_id: this.runId,
      op: 'logger_init'
    });
  }

  generateTxId(): string {
    return Math.random().toString(36).substr(2, 5);
  }

  log(level: LogEntry['level'], message: string, metadata: Record<string, any> = {}) {
    const entry: LogEntry = {
      ts_ns: process.hrtime.bigint().toString(),
      run_id: this.runId,
      tick: this.currentTick,
      tx_id: metadata.tx_id || this.generateTxId(),
      op: metadata.op || 'unknown',
      zone: metadata.zone || 'main',
      level,
      message,
      ...metadata
    };

    this.logs.push(entry);

    // Console output pour développement
    const timestamp = new Date().toISOString();
    console.log(`[${timestamp}] [${level.toUpperCase()}] ${message}`);
  }

  logConversion(inputBits: string, outputLums: any[], metadata: Record<string, any> = {}) {
    const pattern = this.analyzeBitPattern(inputBits);

    this.log('info', 'Bit to LUM conversion', {
      op: 'bit_to_lum_conversion',
      input_length: inputBits.length,
      output_count: outputLums.length,
      bit_pattern: pattern,
      conservation_valid: inputBits.length === outputLums.length,
      ...metadata
    });

    // Log individuel pour chaque LUM (traçabilité LUM-par-LUM)
    outputLums.forEach((lum, index) => {
      this.log('debug', 'LUM created from bit', {
        op: 'lum_creation',
        lum_id: lum.lum_id || `L-${this.runId}-${index.toString().padStart(6, '0')}`,
        bit_position: index,
        bit_value: inputBits[index],
        lum_presence: lum.presence
      });
    });
  }

  logLumOperation(
    operation: string, 
    zone: string, 
    lumIds: string[], 
    prevCount: number, 
    postCount: number, 
    metadata: Record<string, any> = {}
  ) {
    const conservationValid = operation === 'cycle' || prevCount === postCount;

    // Log principal avec validation conservation
    this.log('info', `LUM operation: ${operation}`, {
      op: operation,
      zone,
      prev_count: prevCount,
      post_count: postCount,
      conservation_valid: conservationValid,
      lum_count: lumIds.length,
      ...metadata
    });

    // Log individuel pour chaque LUM (traçabilité LUM-par-LUM)
    lumIds.forEach((lumId, index) => {
      this.log('debug', `LUM ${operation} individual tracking`, {
        op: `${operation}_individual`,
        operation_sequence: `${index + 1}/${lumIds.length}`,
        zone,
        lum_id: lumId,
        ...metadata
      });
    });

    this.currentTick++;
  }

  private analyzeBitPattern(bits: string) {
    const ones = (bits.match(/1/g) || []).length;
    const transitions = (bits.match(/01|10/g) || []).length;

    let patternType = 'MIXED';
    if (ones === 0) patternType = 'ALL_ZEROS';
    else if (ones === bits.length) patternType = 'ALL_ONES';
    else if (transitions === bits.length - 1) patternType = 'ALTERNATING';

    return {
      length: bits.length,
      ones_count: ones,
      zeros_count: bits.length - ones,
      ones_ratio: ones / bits.length,
      transitions,
      pattern_type: patternType
    };
  }

  getStats() {
    const now = process.hrtime.bigint();
    const durationNs = now - this.startTime;
    const durationMs = Number(durationNs / BigInt(1000000));

    const levels = this.logs.reduce((acc, log) => {
      acc[log.level] = (acc[log.level] || 0) + 1;
      return acc;
    }, {} as Record<string, number>);

    const operations = this.logs.reduce((acc, log) => {
      if (log.op !== 'unknown') {
        acc[log.op] = (acc[log.op] || 0) + 1;
      }
      return acc;
    }, {} as Record<string, number>);

    const conservationStats = {
      total_operations: this.logs.filter(log => log.conservation_valid !== undefined).length,
      conserved_operations: this.logs.filter(log => log.conservation_valid === true).length,
      violated_operations: this.logs.filter(log => log.conservation_valid === false).length
    };

    return {
      run_id: this.runId,
      total_logs: this.logs.length,
      current_tick: this.currentTick,
      duration_ms: durationMs,
      levels,
      operations,
      conservation_stats: {
        ...conservationStats,
        conservation_rate: conservationStats.total_operations > 0 ? 
          conservationStats.conserved_operations / conservationStats.total_operations : 0
      }
    };
  }

  saveToJSONL(): string {
    const filename = `${this.runId}.jsonl`;
    const filepath = path.join(process.cwd(), 'logs', filename);

    const jsonlContent = this.logs.map(log => JSON.stringify(log)).join('\n');
    fs.writeFileSync(filepath, jsonlContent, 'utf-8');

    this.log('info', 'Logs saved to JSONL', { 
      op: 'logs_save',
      filepath,
      log_count: this.logs.length
    });

    return filepath;
  }
}

// Singleton instance
export const logger = new LUMLogger();