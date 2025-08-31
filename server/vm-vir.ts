
import { VIRInstruction } from "./services/vorax-compiler";
import { logger } from "./services/logger";

export class VoraxVM {
  private zones: number[] = new Array(26).fill(0); // A-Z zones
  private memory: number[] = new Array(10).fill(0); // Memory slots
  private stack: number[] = [];
  private pc: number = 0; // Program counter
  private running: boolean = false;
  private energyBudget: number = 1000;
  private tickCount: number = 0;

  constructor() {
    this.reset();
  }

  reset(): void {
    this.zones.fill(0);
    this.memory.fill(0);
    this.stack = [];
    this.pc = 0;
    this.running = false;
    this.energyBudget = 1000;
    this.tickCount = 0;
  }

  execute(instructions: VIRInstruction[]): VoraxExecutionResult {
    this.running = true;
    this.pc = 0;
    const executionLog: VoraxTickLog[] = [];

    try {
      while (this.running && this.pc < instructions.length && this.energyBudget > 0) {
        const instruction = instructions[this.pc];
        const tickLog = this.executeInstruction(instruction);
        executionLog.push(tickLog);
        
        this.pc++;
        this.tickCount++;
        this.energyBudget -= this.getInstructionCost(instruction);

        // Log tick-by-tick execution
        logger.logVoraxTick({
          tick: this.tickCount,
          instruction,
          zones: [...this.zones],
          memory: [...this.memory],
          energy: this.energyBudget
        });
      }
    } catch (error) {
      logger.error('VM execution error:', error);
      return {
        success: false,
        error: error instanceof Error ? error.message : 'Unknown VM error',
        finalState: this.getState(),
        executionLog
      };
    }

    return {
      success: true,
      finalState: this.getState(),
      executionLog,
      ticksExecuted: this.tickCount,
      energyUsed: 1000 - this.energyBudget
    };
  }

  private executeInstruction(instruction: VIRInstruction): VoraxTickLog {
    const beforeState = this.getState();
    
    switch (instruction.opcode) {
      case 0x10: // FUSE
        this.executeFuse(instruction.operands[0], instruction.operands[1]);
        break;
      case 0x11: // SPLIT
        this.executeSplit(instruction.operands[0], instruction.operands[1]);
        break;
      case 0x12: // MOVE
        this.executeMove(instruction.operands[0], instruction.operands[1], instruction.operands[2]);
        break;
      case 0x13: // CYCLE
        this.executeCycle(instruction.operands[0], instruction.operands[1]);
        break;
      case 0x14: // STORE
        this.executeStore(instruction.operands[0], instruction.operands[1]);
        break;
      case 0x15: // RETRIEVE
        this.executeRetrieve(instruction.operands[0], instruction.operands[1]);
        break;
      case 0x16: // COMPRESS
        this.executeCompress(instruction.operands[0], instruction.metadata.cost || 5);
        break;
      case 0x17: // EXPAND
        this.executeExpand(instruction.operands[0], instruction.operands[1]);
        break;
      case 0xFF: // HALT
        this.running = false;
        break;
      default:
        logger.warn(`Unknown opcode: 0x${instruction.opcode.toString(16)}`);
    }

    return {
      tick: this.tickCount,
      opcode: instruction.opcode,
      beforeState,
      afterState: this.getState(),
      energyCost: this.getInstructionCost(instruction)
    };
  }

  private executeFuse(zone1: number, zone2: number): void {
    if (zone1 >= 0 && zone1 < this.zones.length && zone2 >= 0 && zone2 < this.zones.length) {
      this.zones[zone1] += this.zones[zone2];
      this.zones[zone2] = 0;
    }
  }

  private executeSplit(zone: number, parts: number): void {
    if (zone >= 0 && zone < this.zones.length && parts > 0) {
      const value = this.zones[zone];
      const perPart = Math.floor(value / parts);
      const remainder = value % parts;
      
      this.zones[zone] = perPart + (remainder > 0 ? 1 : 0);
      
      // Distribute to next available zones
      for (let i = 1; i < parts && zone + i < this.zones.length; i++) {
        this.zones[zone + i] = perPart + (i < remainder ? 1 : 0);
      }
    }
  }

  private executeMove(srcZone: number, dstZone: number, amount: number): void {
    if (srcZone >= 0 && srcZone < this.zones.length && 
        dstZone >= 0 && dstZone < this.zones.length &&
        this.zones[srcZone] >= amount) {
      this.zones[srcZone] -= amount;
      this.zones[dstZone] += amount;
    }
  }

  private executeCycle(zone: number, modulo: number): void {
    if (zone >= 0 && zone < this.zones.length && modulo > 0) {
      this.zones[zone] = this.zones[zone] % modulo;
    }
  }

  private executeStore(memorySlot: number, zone: number): void {
    if (memorySlot >= 0 && memorySlot < this.memory.length &&
        zone >= 0 && zone < this.zones.length) {
      this.memory[memorySlot] = this.zones[zone];
      this.zones[zone] = 0;
    }
  }

  private executeRetrieve(memorySlot: number, zone: number): void {
    if (memorySlot >= 0 && memorySlot < this.memory.length &&
        zone >= 0 && zone < this.zones.length) {
      this.zones[zone] = this.memory[memorySlot];
      this.memory[memorySlot] = 0;
    }
  }

  private executeCompress(zone: number, cost: number): void {
    if (zone >= 0 && zone < this.zones.length && this.energyBudget >= cost) {
      // Compress LUMs into Ω (omega) - represented as negative value
      this.zones[zone] = -Math.abs(this.zones[zone]);
      this.energyBudget -= cost;
    }
  }

  private executeExpand(zone: number, factor: number): void {
    if (zone >= 0 && zone < this.zones.length && factor > 0) {
      // Expand Ω back to LUMs
      if (this.zones[zone] < 0) {
        this.zones[zone] = Math.abs(this.zones[zone]) * factor;
      }
    }
  }

  private getInstructionCost(instruction: VIRInstruction): number {
    switch (instruction.opcode) {
      case 0x10: case 0x11: case 0x12: case 0x13: // Basic operations
        return 1;
      case 0x14: case 0x15: // Memory operations
        return 2;
      case 0x16: // Compress
        return instruction.metadata?.cost || 5;
      case 0x17: // Expand
        return 3;
      default:
        return 0;
    }
  }

  private getState(): VoraxVMState {
    return {
      zones: [...this.zones],
      memory: [...this.memory],
      pc: this.pc,
      energy: this.energyBudget,
      ticks: this.tickCount
    };
  }
}

export interface VoraxVMState {
  zones: number[];
  memory: number[];
  pc: number;
  energy: number;
  ticks: number;
}

export interface VoraxTickLog {
  tick: number;
  opcode: number;
  beforeState: VoraxVMState;
  afterState: VoraxVMState;
  energyCost: number;
}

export interface VoraxExecutionResult {
  success: boolean;
  error?: string;
  finalState: VoraxVMState;
  executionLog: VoraxTickLog[];
  ticksExecuted?: number;
  energyUsed?: number;
}
