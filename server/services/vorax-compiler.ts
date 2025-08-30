import { OperationResult } from "../types/lums";
import { logger } from "./logger";

interface VoraxInstruction {
  opcode: string;
  operands: string[];
  metadata?: any;
}

export class VoraxCompiler {
  compile(statements: any[]): OperationResult {
    try {
      logger.info('Starting VORAX-L compilation', { statementCount: statements.length });
      
      const instructions: VoraxInstruction[] = [];
      
      for (const statement of statements) {
        const instruction = this.compileStatement(statement);
        if (instruction) {
          instructions.push(instruction);
        }
      }
      
      logger.success('VORAX-L compilation completed', { 
        instructionCount: instructions.length 
      });
      
      return {
        success: true,
        data: { instructions }
      };
    } catch (error) {
      const message = error instanceof Error ? error.message : 'Compilation failed';
      logger.error('VORAX-L compilation failed', { error: message });
      return {
        success: false,
        error: message
      };
    }
  }

  private compileStatement(statement: any): VoraxInstruction | null {
    switch (statement.type) {
      case 'zone_declaration':
        return {
          opcode: 'DECLARE_ZONES',
          operands: statement.data.zones,
        };
        
      case 'memory_declaration':
        return {
          opcode: 'DECLARE_MEMORY',
          operands: statement.data.buffers,
        };
        
      case 'assignment':
        return {
          opcode: 'ASSIGN',
          operands: [statement.data.variable, statement.data.value],
        };
        
      case 'operation':
        return this.compileOperation(statement.data);
        
      case 'assertion':
        return {
          opcode: 'ASSERT',
          operands: [statement.data.assertionType, statement.data.condition],
        };
        
      default:
        logger.warning(`Unknown statement type for compilation: ${statement.type}`);
        return null;
    }
  }

  private compileOperation(data: { operation: string; params: any }): VoraxInstruction {
    const { operation, params } = data;
    
    switch (operation) {
      case 'fuse':
        return {
          opcode: 'FUSE',
          operands: [params.zone, params.lumCount?.toString() || '1'],
        };
        
      case 'split':
        return {
          opcode: 'SPLIT',
          operands: [params.source, ...(params.targets || [])],
        };
        
      case 'move':
        return {
          opcode: 'MOVE',
          operands: [
            params.source || '',
            params.target || '',
            params.lumCount?.toString() || '1'
          ],
        };
        
      case 'store':
        return {
          opcode: 'STORE',
          operands: [
            params.source || '',
            params.buffer || '',
            params.lumCount?.toString() || '1'
          ],
        };
        
      case 'retrieve':
        return {
          opcode: 'RETRIEVE',
          operands: [
            params.buffer || '',
            params.target || ''
          ],
        };
        
      case 'cycle':
        return {
          opcode: 'CYCLE',
          operands: [
            params.zone || '',
            params.modulo?.toString() || '3'
          ],
        };
        
      default:
        throw new Error(`Unknown operation for compilation: ${operation}`);
    }
  }

  generateCCode(instructions: VoraxInstruction[]): string {
    const cCode = `
#include "lums.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    VoraxEngine* engine = create_vorax_engine();
    
    ${instructions.map(instr => this.generateCInstruction(instr)).join('\n    ')}
    
    print_vorax_engine_state(engine);
    free_vorax_engine(engine);
    return 0;
}
`;
    
    return cCode;
  }

  private generateCInstruction(instruction: VoraxInstruction): string {
    const { opcode, operands } = instruction;
    
    switch (opcode) {
      case 'DECLARE_ZONES':
        return operands.map(zone => 
          `vorax_add_zone(engine, "${zone}", 0, 0, 100, 100);`
        ).join('\n    ');
        
      case 'FUSE':
        return `// Fusion operation for ${operands[0]}`;
        
      case 'SPLIT':
        return `// Split operation from ${operands[0]}`;
        
      default:
        return `// ${opcode} operation`;
    }
  }
}

export const voraxCompiler = new VoraxCompiler();
