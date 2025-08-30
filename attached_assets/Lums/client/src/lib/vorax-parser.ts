import type { VoraxOperation, LUMGroup } from "@shared/schema";

interface ParsedVorax {
  zones: Record<string, any>;
  memory: Record<string, any>;
  operations: VoraxOperation[];
  errors: string[];
}

export function parseVoraxCode(code: string): ParsedVorax {
  const result: ParsedVorax = {
    zones: {},
    memory: {},
    operations: [],
    errors: [],
  };

  const lines = code.split('\n').map(line => line.trim()).filter(line => line.length > 0);

  for (const line of lines) {
    try {
      if (line.startsWith('Zone ')) {
        parseZoneDeclaration(line, result);
      } else if (line.startsWith('#')) {
        parseMemoryDefinition(line, result);
      } else if (line.includes('⧉') || line.includes('⇅') || line.includes('⟲') || line.includes('→')) {
        parseOperation(line, result);
      } else if (line.startsWith('⇾')) {
        parseFlowInstruction(line, result);
      }
    } catch (error) {
      result.errors.push(`Error parsing line "${line}": ${error instanceof Error ? error.message : 'Unknown error'}`);
    }
  }

  return result;
}

function parseZoneDeclaration(line: string, result: ParsedVorax): void {
  // Parse: "Zone A : ⦿(•••)" or "Zone A, B, C;"
  const zoneMatch = line.match(/Zone\s+([A-Z][^:]*)\s*:\s*(.+)/);
  if (zoneMatch) {
    const zoneName = zoneMatch[1].trim();
    const zoneContent = zoneMatch[2].trim();
    
    const lumCount = countLumsInExpression(zoneContent);
    result.zones[zoneName] = {
      content: zoneContent,
      lumCount,
      type: zoneContent.includes('⦿') ? 'cluster' : 'linear',
    };
  } else {
    // Multiple zone declaration: "Zone A, B, C;"
    const multiZoneMatch = line.match(/Zone\s+([^;]+);?/);
    if (multiZoneMatch) {
      const zoneNames = multiZoneMatch[1].split(',').map(name => name.trim());
      for (const zoneName of zoneNames) {
        result.zones[zoneName] = {
          content: '(vide)',
          lumCount: 0,
          type: 'linear',
        };
      }
    }
  }
}

function parseMemoryDefinition(line: string, result: ParsedVorax): void {
  // Parse: "#graine := ⦿(••)"
  const memMatch = line.match(/#(\w+)\s*:=\s*(.+)/);
  if (memMatch) {
    const memName = memMatch[1];
    const memContent = memMatch[2];
    
    const lumCount = countLumsInExpression(memContent);
    result.memory[memName] = {
      content: memContent,
      lumCount,
      type: memContent.includes('⦿') ? 'cluster' : 'linear',
    };
  }
}

function parseOperation(line: string, result: ParsedVorax): void {
  // Parse operations like: "•• ⧉ ••• → •••••" or "#graine ⧉ • → ⦿(•••)"
  let operation: VoraxOperation | null = null;

  if (line.includes('⧉')) {
    operation = {
      symbol: '⧉',
      name: 'fusion',
      sourceGroups: extractSourceGroups(line, '⧉'),
      targetGroups: extractTargetGroups(line),
    };
  } else if (line.includes('⇅')) {
    const zones = extractZoneCount(line);
    operation = {
      symbol: '⇅',
      name: 'split',
      sourceGroups: extractSourceGroups(line, '⇅'),
      targetGroups: extractTargetGroups(line),
      parameters: { zones },
    };
  } else if (line.includes('⟲')) {
    const modulo = extractModulo(line);
    operation = {
      symbol: '⟲',
      name: 'cycle',
      sourceGroups: extractSourceGroups(line, '⟲'),
      targetGroups: extractTargetGroups(line),
      parameters: { modulo },
    };
  } else if (line.includes('→')) {
    operation = {
      symbol: '→',
      name: 'flow',
      sourceGroups: extractSourceGroups(line, '→'),
      targetGroups: extractTargetGroups(line),
    };
  }

  if (operation) {
    result.operations.push(operation);
  }
}

function parseFlowInstruction(line: string, result: ParsedVorax): void {
  // Parse: "⇾ Zone A reçoit #graine"
  // This is more descriptive text, we can track it as metadata
  result.operations.push({
    symbol: '→',
    name: 'flow',
    sourceGroups: [],
    targetGroups: [],
    parameters: { description: line.substring(1).trim() },
  });
}

function countLumsInExpression(expr: string): number {
  // Count • symbols in expressions like "•••" or "⦿(••••)"
  const matches = expr.match(/•/g);
  return matches ? matches.length : 0;
}

function extractSourceGroups(line: string, operatorSymbol: string): string[] {
  const parts = line.split(operatorSymbol);
  if (parts.length < 2) return [];
  
  const leftPart = parts[0].trim();
  const groups: string[] = [];
  
  // Extract memory references (#name) or LUM patterns (••• or ⦿(•••))
  const memoryRefs = leftPart.match(/#\w+/g);
  if (memoryRefs) {
    groups.push(...memoryRefs);
  }
  
  const lumPatterns = leftPart.match(/[•⦿()]+/g);
  if (lumPatterns) {
    groups.push(...lumPatterns);
  }
  
  return groups;
}

function extractTargetGroups(line: string): string[] {
  const arrowIndex = line.indexOf('→');
  if (arrowIndex === -1) return [];
  
  const rightPart = line.substring(arrowIndex + 1).trim();
  const groups: string[] = [];
  
  // Extract zone names or LUM patterns
  const zoneRefs = rightPart.match(/Zone\s+[A-Z]/g);
  if (zoneRefs) {
    groups.push(...zoneRefs);
  }
  
  const lumPatterns = rightPart.match(/[•⦿()]+/g);
  if (lumPatterns) {
    groups.push(...lumPatterns);
  }
  
  return groups;
}

function extractZoneCount(line: string): number {
  // Extract number from "⇅ 2" or "⇅ 3 zones"
  const match = line.match(/⇅\s*(\d+)/);
  return match ? parseInt(match[1], 10) : 2;
}

function extractModulo(line: string): number {
  // Extract number from "⟲ 3" or "% 3"
  const match = line.match(/(?:⟲|%)\s*(\d+)/);
  return match ? parseInt(match[1], 10) : 3;
}

export function validateVoraxSyntax(code: string): { valid: boolean; errors: string[] } {
  const parsed = parseVoraxCode(code);
  return {
    valid: parsed.errors.length === 0,
    errors: parsed.errors,
  };
}

export function formatVoraxCode(code: string): string {
  // Basic formatting for VORAX code
  return code
    .split('\n')
    .map(line => line.trim())
    .filter(line => line.length > 0)
    .join('\n');
}
