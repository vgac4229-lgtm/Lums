const { execSync } = require('child_process');
const fs = require('fs');
const path = require('path');

describe('LUMS/VORAX System Tests', () => {
  const API_BASE = 'http://localhost:5000/api';

  beforeAll(() => {
    // Ensure C library is built
    try {
      execSync('make clean && make', { stdio: 'pipe' });
    } catch (error) {
      console.warn('C build failed, tests may be limited');
    }
  });

  // Triple execution test as required
  function tripleTest(testName, testFn) {
    describe(`${testName} (Triple Execution)`, () => {
      test('Execution 1', async () => { await testFn(1); });
      test('Execution 2', async () => { await testFn(2); });
      test('Execution 3', async () => { await testFn(3); });
    });
  }

  tripleTest('Bit to LUM conversion conservation', async (iteration) => {
    const testBits = `110100${iteration.toString().padStart(3, '0')}`;

    const response = await fetch(`${API_BASE}/convert/bit-to-lum`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ inputBits: testBits })
    });

    const result = await response.json();
    expect(result.outputLums).toHaveLength(testBits.length);

    // Verify each LUM corresponds to correct bit
    for (let i = 0; i < testBits.length; i++) {
      expect(result.outputLums[i].presence).toBe(parseInt(testBits[i]));
    }

    // Test reverse conversion
    const reverseResponse = await fetch(`${API_BASE}/convert/lum-to-bit`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ inputLums: result.outputLums })
    });

    const reverseResult = await reverseResponse.json();
    expect(reverseResult.outputBits).toBe(testBits);
  });

  tripleTest('VORAX operations conservation', async (iteration) => {
    // Test fusion operation
    const group1 = { lums: [{ presence: 1 }, { presence: 1 }] };
    const group2 = { lums: [{ presence: 1 }, { presence: 0 }, { presence: 1 }] };

    const response = await fetch(`${API_BASE}/execute/vorax-operation`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        operation: { name: 'fusion' },
        groups: [group1, group2]
      })
    });

    const result = await response.json();
    const totalBefore = group1.lums.length + group2.lums.length;
    const totalAfter = result.result.lums.length;

    expect(totalAfter).toBe(totalBefore);

    // Verify presence conservation
    const presenceBefore = group1.lums.reduce((sum, lum) => sum + lum.presence, 0) +
                          group2.lums.reduce((sum, lum) => sum + lum.presence, 0);
    const presenceAfter = result.result.lums.reduce((sum, lum) => sum + lum.presence, 0);

    expect(presenceAfter).toBe(presenceBefore);
  });

  tripleTest('VORAX split operation', async (iteration) => {
    const sourceGroup = {
      lums: Array(8).fill().map((_, i) => ({ presence: i % 2 }))
    };

    const response = await fetch(`${API_BASE}/execute/vorax-operation`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        operation: { name: 'split', parameters: { zones: 3 } },
        groups: [sourceGroup]
      })
    });

    const result = await response.json();
    const splitGroups = result.result;

    // Verify total LUM count preserved
    const totalBefore = sourceGroup.lums.length;
    const totalAfter = splitGroups.reduce((sum, group) => sum + group.lums.length, 0);
    expect(totalAfter).toBe(totalBefore);

    // Verify equitable distribution
    const expectedSize = Math.floor(totalBefore / 3);
    const remainder = totalBefore % 3;

    splitGroups.forEach((group, index) => {
      const expectedGroupSize = expectedSize + (index < remainder ? 1 : 0);
      expect(group.lums.length).toBe(expectedGroupSize);
    });
  });

  tripleTest('C library integration', async (iteration) => {
    try {
      const testBinary = '1101001010';
      const cTestCode = `
#include "server/lums/lums.h"
#include <stdio.h>

int main() {
    LUMGroup* group = encode_binary_string("${testBinary}");
    if (!group) return 1;

    char output[32];
    if (decode_lum_group_to_binary(group, output, sizeof(output)) < 0) {
        return 1;
    }

    printf("%s", output);
    return 0;
}`;

      fs.writeFileSync('/tmp/test_c_integration.c', cTestCode);

      const compileCmd = 'gcc -I. /tmp/test_c_integration.c build/liblums.a -o /tmp/test_vorax';
      execSync(compileCmd, { stdio: 'pipe' });

      const output = execSync('/tmp/test_vorax', { encoding: 'utf8' });
      expect(output.trim()).toBe(testBinary);

    } catch (error) {
      throw new Error(`C integration test failed: ${error.message}`);
    }
  });

  test('Bootstrap sequence', async () => {
    const phases = [
      'vorax_bootstrap_phase1',
      'vorax_bootstrap_phase2',
      'vorax_bootstrap_phase3',
      'vorax_bootstrap_phase4',
      'vorax_bootstrap_phase5',
      'vorax_bootstrap_phase6'
    ];

    // Test each bootstrap phase
    for (const phase of phases) {
      const response = await fetch(`${API_BASE}/bootstrap/${phase}`, {
        method: 'POST'
      });

      expect(response.ok).toBe(true);
      const result = await response.json();
      expect(result.success).toBe(true);
    }
  });

  test('Linear type checking', async () => {
    // Test that duplication is prevented
    const group = { lums: [{ presence: 1, id: 'lum-unique-123' }] };

    const response1 = await fetch(`${API_BASE}/linear-type/check-duplication`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ group, operation: 'duplicate' })
    });

    const result1 = await response1.json();
    expect(result1.allowed).toBe(false);
    expect(result1.violation).toContain('duplication');
  });

  test('JSONL logging', async () => {
    const response = await fetch(`${API_BASE}/logs/recent`, {
      method: 'GET'
    });

    const logs = await response.json();
    expect(Array.isArray(logs)).toBe(true);

    // Verify JSONL format
    logs.forEach(log => {
      expect(log).toHaveProperty('ts_ns');
      expect(log).toHaveProperty('tick');
      expect(log).toHaveProperty('op');
      expect(typeof log.ts_ns).toBe('number');
    });
  });
});

// Performance benchmark tests
describe('LUMS Performance Tests', () => {
  test('Large scale conversion performance', async () => {
    const largeBinary = '1'.repeat(10000) + '0'.repeat(10000);
    const startTime = Date.now();

    const response = await fetch(`${API_BASE}/convert/bit-to-lum`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ inputBits: largeBinary })
    });

    const endTime = Date.now();
    const result = await response.json();

    expect(response.ok).toBe(true);
    expect(result.outputLums).toHaveLength(20000);
    expect(endTime - startTime).toBeLessThan(1000); // Should complete in under 1s
  });

  test('Memory usage stability', async () => {
    const iterations = 100;
    const initialMemory = process.memoryUsage().heapUsed;

    for (let i = 0; i < iterations; i++) {
      await fetch(`${API_BASE}/convert/bit-to-lum`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ inputBits: '10101010' })
      });
    }

    // Force garbage collection if available
    if (global.gc) global.gc();

    const finalMemory = process.memoryUsage().heapUsed;
    const memoryIncrease = finalMemory - initialMemory;

    // Memory increase should be reasonable (less than 50MB)
    expect(memoryIncrease).toBeLessThan(50 * 1024 * 1024);
  });
});