export interface VoraxToken {
  type: 'keyword' | 'identifier' | 'operator' | 'literal' | 'symbol';
  value: string;
  line: number;
  column: number;
}

export interface VoraxStatement {
  type: 'zone_declaration' | 'memory_declaration' | 'assignment' | 'operation' | 'assertion';
  data: any;
}

export class VoraxParser {
  private tokens: VoraxToken[] = [];
  private position = 0;

  parse(code: string): VoraxStatement[] {
    this.tokens = this.tokenize(code);
    this.position = 0;
    
    const statements: VoraxStatement[] = [];
    
    while (!this.isAtEnd()) {
      const statement = this.parseStatement();
      if (statement) {
        statements.push(statement);
      }
    }
    
    return statements;
  }

  private tokenize(code: string): VoraxToken[] {
    const tokens: VoraxToken[] = [];
    const lines = code.split('\n');
    
    for (let lineNum = 0; lineNum < lines.length; lineNum++) {
      const line = lines[lineNum];
      let column = 0;
      
      while (column < line.length) {
        const char = line[column];
        
        // Skip whitespace
        if (/\s/.test(char)) {
          column++;
          continue;
        }
        
        // Skip comments
        if (line.substring(column, column + 2) === '//') {
          break;
        }
        
        // Parse keywords and identifiers
        if (/[a-zA-Z_]/.test(char)) {
          const start = column;
          while (column < line.length && /[a-zA-Z0-9_]/.test(line[column])) {
            column++;
          }
          const value = line.substring(start, column);
          const type = this.isKeyword(value) ? 'keyword' : 'identifier';
          tokens.push({ type, value, line: lineNum, column: start });
          continue;
        }
        
        // Parse operators and symbols
        if (this.isOperatorStart(char)) {
          const start = column;
          const operator = this.parseOperator(line, column);
          column += operator.length;
          tokens.push({ type: 'operator', value: operator, line: lineNum, column: start });
          continue;
        }
        
        // Parse symbols
        if (this.isSymbol(char)) {
          tokens.push({ type: 'symbol', value: char, line: lineNum, column });
          column++;
          continue;
        }
        
        column++;
      }
    }
    
    return tokens;
  }

  private parseStatement(): VoraxStatement | null {
    if (this.isAtEnd()) return null;
    
    const token = this.peek();
    
    switch (token.value) {
      case 'zone':
        return this.parseZoneDeclaration();
      case 'mem':
        return this.parseMemoryDeclaration();
      case 'fuse':
      case 'split':
      case 'move':
      case 'store':
      case 'retrieve':
      case 'cycle':
        return this.parseOperation();
      case 'assert':
        return this.parseAssertion();
      default:
        if (this.peekNext()?.value === ':=') {
          return this.parseAssignment();
        }
        this.advance(); // Skip unknown tokens
        return null;
    }
  }

  private parseZoneDeclaration(): VoraxStatement {
    this.advance(); // consume 'zone'
    const zones: string[] = [];
    
    while (!this.isAtEnd() && this.peek().value !== ';') {
      if (this.peek().type === 'identifier') {
        zones.push(this.advance().value);
      } else if (this.peek().value === ',') {
        this.advance(); // consume comma
      } else {
        break;
      }
    }
    
    if (this.peek().value === ';') {
      this.advance(); // consume semicolon
    }
    
    return {
      type: 'zone_declaration',
      data: { zones }
    };
  }

  private parseMemoryDeclaration(): VoraxStatement {
    this.advance(); // consume 'mem'
    const buffers: string[] = [];
    
    while (!this.isAtEnd() && this.peek().value !== ';') {
      if (this.peek().type === 'identifier') {
        buffers.push(this.advance().value);
      } else if (this.peek().value === ',') {
        this.advance(); // consume comma
      } else {
        break;
      }
    }
    
    if (this.peek().value === ';') {
      this.advance(); // consume semicolon
    }
    
    return {
      type: 'memory_declaration',
      data: { buffers }
    };
  }

  private parseOperation(): VoraxStatement {
    const operation = this.advance().value;
    const params: any = {};
    
    // Parse operation parameters based on type
    switch (operation) {
      case 'fuse':
        params.zone = this.advance().value;
        if (this.peek().value === ',') {
          this.advance(); // consume comma
          params.lumCount = this.parseLumCount();
        }
        break;
      case 'split':
        params.source = this.advance().value;
        if (this.peek().value === '->') {
          this.advance(); // consume '->'
          params.targets = this.parseTargetList();
        }
        break;
      // Add other operation parsers...
    }
    
    this.consumeStatementEnd();
    
    return {
      type: 'operation',
      data: { operation, params }
    };
  }

  private parseAssignment(): VoraxStatement {
    const variable = this.advance().value;
    this.advance(); // consume ':='
    
    // Parse the right-hand side
    const value = this.parseExpression();
    
    this.consumeStatementEnd();
    
    return {
      type: 'assignment',
      data: { variable, value }
    };
  }

  private parseAssertion(): VoraxStatement {
    this.advance(); // consume 'assert'
    const assertionType = this.advance().value;
    
    this.advance(); // consume ':'
    
    const condition = this.parseExpression();
    
    this.consumeStatementEnd();
    
    return {
      type: 'assertion',
      data: { assertionType, condition }
    };
  }

  private parseExpression(): any {
    // Simple expression parser - can be extended
    const tokens: string[] = [];
    
    while (!this.isAtEnd() && this.peek().value !== ';') {
      tokens.push(this.advance().value);
    }
    
    return tokens.join(' ');
  }

  private parseLumCount(): number {
    const token = this.peek();
    if (token.value.includes('•')) {
      return token.value.length; // Count bullets
    }
    return parseInt(token.value) || 1;
  }

  private parseTargetList(): string[] {
    const targets: string[] = [];
    
    if (this.peek().value === '[') {
      this.advance(); // consume '['
      
      while (!this.isAtEnd() && this.peek().value !== ']') {
        if (this.peek().type === 'identifier') {
          targets.push(this.advance().value);
        } else if (this.peek().value === ',') {
          this.advance(); // consume comma
        } else {
          break;
        }
      }
      
      if (this.peek().value === ']') {
        this.advance(); // consume ']'
      }
    }
    
    return targets;
  }

  private consumeStatementEnd(): void {
    if (this.peek().value === ';') {
      this.advance();
    }
  }

  private isKeyword(value: string): boolean {
    const keywords = ['zone', 'mem', 'fuse', 'split', 'move', 'store', 'retrieve', 'cycle', 'assert', 'emit', 'group'];
    return keywords.includes(value);
  }

  private isOperatorStart(char: string): boolean {
    return '+-*/%=<>!&|:'.includes(char);
  }

  private parseOperator(line: string, start: number): string {
    const twoCharOps = [':=', '->', '==', '!=', '<=', '>=', '&&', '||', '+='];
    
    for (const op of twoCharOps) {
      if (line.substring(start, start + op.length) === op) {
        return op;
      }
    }
    
    return line[start];
  }

  private isSymbol(char: string): boolean {
    return '()[]{},.;•'.includes(char);
  }

  private peek(): VoraxToken {
    if (this.isAtEnd()) {
      return { type: 'symbol', value: '', line: 0, column: 0 };
    }
    return this.tokens[this.position];
  }

  private peekNext(): VoraxToken | null {
    if (this.position + 1 >= this.tokens.length) {
      return null;
    }
    return this.tokens[this.position + 1];
  }

  private advance(): VoraxToken {
    if (!this.isAtEnd()) {
      this.position++;
    }
    return this.tokens[this.position - 1];
  }

  private isAtEnd(): boolean {
    return this.position >= this.tokens.length;
  }
}
