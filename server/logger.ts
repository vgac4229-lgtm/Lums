

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
  public runId: string;
  private currentTick: number = 0;
  private logsDir: string;
  private sessionStartTime: bigint;

  constructor() {
    this.runId = `run-${Date.now()}-${Math.random().toString(36).substr(2, 9)}`;
    this.logsDir = path.join(process.cwd(), 'logs');
    this.sessionStartTime = process.hrtime.bigint();
    this.ensureLogsDirectory();
    
    // Log initial de session
    this.log('info', 'LUMS Logger session started', {
      op: 'session_start',
      run_id: this.runId,
      timestamp: new Date().toISOString(),
      node_version: process.version,
      platform: process.platform
    });
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
    this.log('debug', `Tick incremented to ${this.currentTick}`, {
      op: 'tick_increment',
      new_tick: this.currentTick
    });
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

    // Log immédiat pour debug avec couleurs
    if (process.env.NODE_ENV === 'development') {
      const colors = {
        debug: '\x1b[36m',    // Cyan
        info: '\x1b[32m',     // Green
        warn: '\x1b[33m',     // Yellow
        error: '\x1b[31m',    // Red
        success: '\x1b[35m'   // Magenta
      };
      const reset = '\x1b[0m';
      const timestamp = new Date().toISOString().substring(11, 23); // HH:mm:ss.sss
      
      console.log(
        `${colors[level]}[${level.toUpperCase().padEnd(7)}] ${timestamp} ${message}${reset}`,
        entry.metadata ? JSON.stringify(entry.metadata, null, 2) : ''
      );
    }
  }

  // Logs spécialisés pour les opérations LUM avec conservation tracking
  logLumOperation(
    op: string,
    zone: string,
    lumIds: string[],
    prevCount: number,
    postCount: number,
    metadata: Record<string, any> = {}
  ): void {
    const txId = this.generateTxId();
    const conservationValid = op === 'cycle' || prevCount === postCount;
    
    // Log principal de l'opération avec conservation tracking
    this.log('info', `LUM operation: ${op}`, {
      op,
      zone,
      tx_id: txId,
      prev_count: prevCount,
      post_count: postCount,
      added: Math.max(0, postCount - prevCount),
      removed: Math.max(0, prevCount - postCount),
      conservation_valid: conservationValid,
      lum_count: lumIds.length,
      metadata: {
        operation_type: op,
        conservation_law: conservationValid ? 'RESPECTED' : 'VIOLATED',
        ...metadata
      }
    });

    // Log individuel pour chaque LUM affecté (traçabilité LUM-par-LUM)
    lumIds.forEach((lumId, index) => {
      this.log('debug', `LUM ${op} individual tracking`, {
        op: `${op}_individual`,
        zone,
        lum_id: lumId,
        tx_id: txId,
        lum_index: index,
        total_lums: lumIds.length,
        operation_sequence: `${index + 1}/${lumIds.length}`,
        metadata: {
          individual_tracking: true,
          parent_operation: op,
          ...metadata
        }
      });
    });

    // Log de conservation si violation détectée
    if (!conservationValid) {
      this.log('error', `Conservation law violation in ${op}`, {
        op: 'conservation_violation',
        zone,
        tx_id: txId,
        prev_count: prevCount,
        post_count: postCount,
        difference: postCount - prevCount,
        violation_type: postCount > prevCount ? 'CREATION' : 'DESTRUCTION',
        metadata: {
          violation_severity: 'CRITICAL',
          requires_investigation: true,
          ...metadata
        }
      });
    }
  }

  logConversion(inputBits: string, outputLums: any[], metadata: Record<string, any> = {}): void {
    const txId = this.generateTxId();
    const conversionRatio = outputLums.length / inputBits.length;
    
    this.log('info', 'Bit to LUM conversion completed', {
      op: 'conversion',
      tx_id: txId,
      prev_count: 0,
      post_count: outputLums.length,
      added: outputLums.length,
      metadata: {
        input_bits: inputBits,
        input_length: inputBits.length,
        output_length: outputLums.length,
        conversion_ratio: conversionRatio,
        bit_pattern: this.analyzeBitPattern(inputBits),
        lum_distribution: this.analyzeLumDistribution(outputLums),
        ...metadata
      }
    });

    // Log pour chaque LUM créé avec position et contexte
    outputLums.forEach((lum, index) => {
      this.log('debug', 'LUM created from bit', {
        op: 'bit_to_lum',
        tx_id: txId,
        lum_id: lum.lum_id || `L-${this.runId}-${index.toString().padStart(6, '0')}`,
        metadata: {
          bit_position: index,
          bit_value: inputBits[index],
          lum_presence: lum.presence,
          spatial_position: lum.position,
          structure_type: lum.structureType || lum.structure_type,
          conversion_context: 'bit_to_lum_individual',
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
        validation_timestamp: new Date().toISOString(),
        ...metadata
      }
    });

    // Log détaillé pour chaque erreur
    if (!success && errors.length > 0) {
      errors.forEach((error, index) => {
        this.log('warn', `Validation error ${index + 1}`, {
          op: 'validation_error',
          metadata: {
            target,
            error_index: index + 1,
            error_message: error,
            total_errors: errors.length
          }
        });
      });
    }
  }

  // Analyse des patterns de bits
  private analyzeBitPattern(bits: string): Record<string, any> {
    const ones = (bits.match(/1/g) || []).length;
    const zeros = bits.length - ones;
    const transitions = (bits.match(/01|10/g) || []).length;
    
    return {
      length: bits.length,
      ones_count: ones,
      zeros_count: zeros,
      ones_ratio: ones / bits.length,
      transitions: transitions,
      complexity: transitions / Math.max(1, bits.length - 1),
      pattern_type: this.classifyBitPattern(bits)
    };
  }

  private classifyBitPattern(bits: string): string {
    if (/^0+$/.test(bits)) return 'ALL_ZEROS';
    if (/^1+$/.test(bits)) return 'ALL_ONES';
    if (/^(01)+0?$/.test(bits)) return 'ALTERNATING_01';
    if (/^(10)+1?$/.test(bits)) return 'ALTERNATING_10';
    if (bits.length <= 4) return 'SHORT_PATTERN';
    return 'COMPLEX_PATTERN';
  }

  // Analyse de la distribution des LUMs
  private analyzeLumDistribution(lums: any[]): Record<string, any> {
    const presenceCount = lums.filter(l => l.presence === 1).length;
    const absenceCount = lums.length - presenceCount;
    
    const structures = lums.reduce((acc, lum) => {
      const type = lum.structureType || lum.structure_type || 'unknown';
      acc[type] = (acc[type] || 0) + 1;
      return acc;
    }, {});

    return {
      total_lums: lums.length,
      presence_count: presenceCount,
      absence_count: absenceCount,
      presence_ratio: presenceCount / lums.length,
      structure_distribution: structures,
      spatial_span: this.calculateSpatialSpan(lums)
    };
  }

  private calculateSpatialSpan(lums: any[]): Record<string, any> {
    if (lums.length === 0) return { min_x: 0, max_x: 0, min_y: 0, max_y: 0, span_x: 0, span_y: 0 };
    
    const positions = lums.map(l => l.position).filter(p => p && typeof p.x === 'number' && typeof p.y === 'number');
    if (positions.length === 0) return { min_x: 0, max_x: 0, min_y: 0, max_y: 0, span_x: 0, span_y: 0 };
    
    const xs = positions.map(p => p.x);
    const ys = positions.map(p => p.y);
    
    return {
      min_x: Math.min(...xs),
      max_x: Math.max(...xs),
      min_y: Math.min(...ys),
      max_y: Math.max(...ys),
      span_x: Math.max(...xs) - Math.min(...xs),
      span_y: Math.max(...ys) - Math.min(...ys),
      total_positions: positions.length
    };
  }

  saveToJSONL(): string {
    const timestamp = new Date().toISOString().replace(/[:.]/g, '-');
    const filename = `${this.runId}-${timestamp}.jsonl`;
    const filepath = path.join(this.logsDir, filename);

    // Ajout d'un log de session final
    this.log('info', 'Saving session logs to JSONL', {
      op: 'save_logs',
      metadata: {
        total_logs: this.logs.length,
        session_duration_ns: (process.hrtime.bigint() - this.sessionStartTime).toString(),
        final_tick: this.currentTick,
        filepath
      }
    });

    const jsonlContent = this.logs.map(log => JSON.stringify(log)).join('\n');
    fs.writeFileSync(filepath, jsonlContent);

    return filepath;
  }

  getStats(): Record<string, any> {
    const sessionDurationNs = process.hrtime.bigint() - this.sessionStartTime;
    const sessionDurationMs = Number(sessionDurationNs / BigInt(1000000));
    
    const stats = {
      total_logs: this.logs.length,
      run_id: this.runId,
      current_tick: this.currentTick,
      session_duration_ms: sessionDurationMs,
      session_duration_ns: sessionDurationNs.toString(),
      levels: {} as Record<string, number>,
      operations: {} as Record<string, number>,
      zones: {} as Record<string, number>,
      conservation_stats: {
        total_operations: 0,
        conserved_operations: 0,
        violated_operations: 0
      },
      performance: {
        logs_per_second: Math.round((this.logs.length / sessionDurationMs) * 1000),
        avg_logs_per_tick: this.currentTick > 0 ? Math.round(this.logs.length / this.currentTick) : 0
      }
    };

    // Statistiques par niveau et opération
    this.logs.forEach(log => {
      stats.levels[log.level] = (stats.levels[log.level] || 0) + 1;
      stats.operations[log.op] = (stats.operations[log.op] || 0) + 1;
      
      if (log.zone) {
        stats.zones[log.zone] = (stats.zones[log.zone] || 0) + 1;
      }

      // Tracking de conservation
      if (log.metadata?.conservation_valid !== undefined) {
        stats.conservation_stats.total_operations++;
        if (log.metadata.conservation_valid) {
          stats.conservation_stats.conserved_operations++;
        } else {
          stats.conservation_stats.violated_operations++;
        }
      }
    });

    // Calcul du taux de conservation
    if (stats.conservation_stats.total_operations > 0) {
      stats.conservation_stats.conservation_rate = 
        stats.conservation_stats.conserved_operations / stats.conservation_stats.total_operations;
    }

    return stats;
  }

  // Recherche dans les logs
  searchLogs(query: { op?: string; level?: string; lum_id?: string; zone?: string }): LUMLogEntry[] {
    return this.logs.filter(log => {
      if (query.op && log.op !== query.op) return false;
      if (query.level && log.level !== query.level) return false;
      if (query.lum_id && log.lum_id !== query.lum_id) return false;
      if (query.zone && log.zone !== query.zone) return false;
      return true;
    });
  }

  flush(): void {
    this.log('info', 'Flushing logger session', {
      op: 'session_flush',
      metadata: {
        logs_flushed: this.logs.length,
        final_tick: this.currentTick
      }
    });
    
    this.logs = [];
    this.currentTick = 0;
    this.sessionStartTime = process.hrtime.bigint();
  }
}

// Instance singleton
export const logger = new LUMLogger();

// Export du type pour utilisation externe
export type { LUMLogEntry };

