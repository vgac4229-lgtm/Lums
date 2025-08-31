
import { VoraxOperation, LUMGroup, VoraxEngineState } from "@shared/schema";
import { logger } from "./logger";

export interface VIRInstruction {
  opcode: number;
  operands: number[];
  metadata: Record<string, any>;
}

export class VoraxCompiler {
  private instructions: VIRInstruction[] = [];
  private labelMap: Map<string, number> = new Map();
  
  // Opcodes V-IR
  private static readonly OPCODES = {
    FUSE: 0x10,
    SPLIT: 0x11,
    MOVE: 0x12,
    CYCLE: 0x13,
    STORE: 0x14,
    RETRIEVE: 0x15,
    COMPRESS: 0x16,
    EXPAND: 0x17,
    NOP: 0x00,
    HALT: 0xFF
  };

  private tokenizeLine(line: string): string[] {
    const tokens: string[] = [];
    let current = '';
    let inQuotes = false;
    let inParens = false;
    
    for (let i = 0; i < line.length; i++) {
      const char = line[i];
      
      if (char === '"' && !inParens) {
        inQuotes = !inQuotes;
        continue;
      }
      
      if (char === '(' && !inQuotes) {
        inParens = true;
        current += char;
        continue;
      }
      
      if (char === ')' && !inQuotes) {
        inParens = false;
        current += char;
        continue;
      }
      
      if ((char === ' ' || char === '\t') && !inQuotes && !inParens) {
        if (current.trim()) {
          tokens.push(current.trim());
          current = '';
        }
      } else {
        current += char;
      }
    }
    
    if (current.trim()) {
      tokens.push(current.trim());
    }
    
    return tokens;
  }

  private parseExpression(expr: string): number {
    // Parse des expressions arithmétiques simples
    expr = expr.trim();
    
    // Support des parenthèses
    if (expr.startsWith('(') && expr.endsWith(')')) {
      return this.parseExpression(expr.slice(1, -1));
    }
    
    // Support des opérations de base
    const addMatch = expr.match(/^(.+)\+(.+)$/);
    if (addMatch) {
      return this.parseExpression(addMatch[1]) + this.parseExpression(addMatch[2]);
    }
    
    const subMatch = expr.match(/^(.+)\-(.+)$/);
    if (subMatch) {
      return this.parseExpression(subMatch[1]) - this.parseExpression(subMatch[2]);
    }
    
    const mulMatch = expr.match(/^(.+)\*(.+)$/);
    if (mulMatch) {
      return this.parseExpression(mulMatch[1]) * this.parseExpression(mulMatch[2]);
    }
    
    const divMatch = expr.match(/^(.+)\/(.+)$/);
    if (divMatch) {
      const denominator = this.parseExpression(divMatch[2]);
      if (denominator === 0) throw new Error('Division by zero');
      return Math.floor(this.parseExpression(divMatch[1]) / denominator);
    }
    
    // Nombres littéraux
    const num = parseInt(expr);
    if (!isNaN(num)) {
      return num;
    }
    
    throw new Error(`Cannot parse expression: ${expr}`);
  }

    CYCLE: 0x13,
    STORE: 0x14,
    RETRIEVE: 0x15,
    COMPRESS: 0x16,
    EXPAND: 0x17,
    NOP: 0x00,
    HALT: 0xFF
  };

  parseVoraxCode(code: string): VIRInstruction[] {
    const lines = code.split('\n').filter(line => line.trim() && !line.startsWith('#'));
    this.instructions = [];
    this.labelMap.clear();

    // First pass: collect labels
    lines.forEach((line, index) => {
      const trimmed = line.trim();
      if (trimmed.endsWith(':')) {
        this.labelMap.set(trimmed.slice(0, -1), this.instructions.length);
      }
    });

    // Second pass: compile instructions
    lines.forEach(line => {
      const trimmed = line.trim();
      if (!trimmed.endsWith(':')) {
        this.compileLine(trimmed);
      }
    });

    this.instructions.push({
      opcode: VoraxCompiler.OPCODES.HALT,
      operands: [],
      metadata: {}
    });

    return this.instructions;
  }

  private compileLine(line: string): void {
    // Tokenisation avancée avec support des expressions
    const tokens = this.tokenizeLine(line);
    if (tokens.length === 0) return;
    
    const command = tokens[0].toLowerCase();

    switch (command) {
      case 'fuse':
        this.instructions.push({
          opcode: VoraxCompiler.OPCODES.FUSE,
          operands: [this.parseZone(tokens[1]), this.parseZone(tokens[2])],
          metadata: { sourceZones: [tokens[1], tokens[2]] }
        });
        break;

      case 'split':
        const zones = parseInt(tokens[3]) || 2;
        this.instructions.push({
          opcode: VoraxCompiler.OPCODES.SPLIT,
          operands: [this.parseZone(tokens[1]), zones],
          metadata: { sourceZone: tokens[1], targetZones: zones }
        });
        break;

      case 'move':
        const amount = parseInt(tokens[4]) || 1;
        this.instructions.push({
          opcode: VoraxCompiler.OPCODES.MOVE,
          operands: [this.parseZone(tokens[1]), this.parseZone(tokens[3]), amount],
          metadata: { sourceZone: tokens[1], targetZone: tokens[3], amount }
        });
        break;

      case 'cycle':
        const modulo = parseInt(tokens[3]) || 3;
        this.instructions.push({
          opcode: VoraxCompiler.OPCODES.CYCLE,
          operands: [this.parseZone(tokens[1]), modulo],
          metadata: { zone: tokens[1], modulo }
        });
        break;

      case 'store':
        this.instructions.push({
          opcode: VoraxCompiler.OPCODES.STORE,
          operands: [this.parseMemory(tokens[1]), this.parseZone(tokens[3])],
          metadata: { memory: tokens[1], sourceZone: tokens[3] }
        });
        break;

      case 'retrieve':
        this.instructions.push({
          opcode: VoraxCompiler.OPCODES.RETRIEVE,
          operands: [this.parseMemory(tokens[1]), this.parseZone(tokens[3])],
          metadata: { memory: tokens[1], targetZone: tokens[3] }
        });
        break;

      default:
        logger.warn(`Unknown VORAX command: ${command}`);
        this.instructions.push({
          opcode: VoraxCompiler.OPCODES.NOP,
          operands: [],
          metadata: { originalLine: line }
        });
    }
  }

  private parseZone(zoneName: string): number {
    // Parsing réel des noms de zones
    const zonePattern = /^Zone_([A-Z]+)$/i;
    const match = zoneName.match(zonePattern);
    if (match) {
      let result = 0;
      const letters = match[1].toUpperCase();
      for (let i = 0; i < letters.length; i++) {
        result = result * 26 + (letters.charCodeAt(i) - 65 + 1);
      }
      return result - 1; // 0-indexed
    }
    
    // Fallback pour noms simples A, B, C...
    if (zoneName.length === 1 && /[A-Z]/i.test(zoneName)) {
      return zoneName.toUpperCase().charCodeAt(0) - 65;
    }
    
    throw new Error(`Invalid zone name: ${zoneName}`);
  }

  private parseMemory(memName: string): number {
    // Parsing réel des adresses mémoire
    const memPattern = /^#([A-Za-z0-9_]+)$/;
    const match = memName.match(memPattern);
    if (match) {
      // Hash CRC32 simplifié pour les noms de mémoire
      let hash = 0;
      const name = match[1];
      for (let i = 0; i < name.length; i++) {
        const char = name.charCodeAt(i);
        hash = ((hash << 5) - hash) + char;
        hash = hash & hash; // Convert to 32-bit integer
      }
      return Math.abs(hash) % 256; // Limité à 256 slots mémoire
    }
    
    throw new Error(`Invalid memory name: ${memName}`);
  }

  generateCCode(instructions: VIRInstruction[]): string {
    let cCode = `// Generated VORAX C code
#include "lums/lums.h"
#include <stdio.h>
#include <stdlib.h>

int execute_vorax_program() {
    VoraxEngine* engine = vorax_create_engine();
    if (!engine) return -1;
    
`;

    instructions.forEach((instr, index) => {
      switch (instr.opcode) {
        case VoraxCompiler.OPCODES.FUSE:
          cCode += `    // FUSE zones ${instr.metadata.sourceZones.join(', ')}\n`;
          cCode += `    vorax_fuse_zones(engine, ${instr.operands[0]}, ${instr.operands[1]});\n`;
          break;
        case VoraxCompiler.OPCODES.SPLIT:
          cCode += `    // SPLIT zone ${instr.metadata.sourceZone} into ${instr.operands[1]} parts\n`;
          cCode += `    vorax_split_zone(engine, ${instr.operands[0]}, ${instr.operands[1]});\n`;
          break;
        case VoraxCompiler.OPCODES.HALT:
          cCode += `    // HALT\n    break;\n`;
          break;
      }
    });

    cCode += `
    vorax_destroy_engine(engine);
    return 0;
}
`;

    return cCode;
  }
}
