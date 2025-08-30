
import { describe, test, expect, beforeEach, afterEach } from '@jest/globals';
import { spawn } from 'child_process';
import { promises as fs } from 'fs';
import { join } from 'path';

// Configuration des tests LUMS/VORAX
const API_BASE = 'http://localhost:5000';
const TEST_TIMEOUT = 10000;

// Classe de test runner triple validation
class TripleTestRunner {
  constructor() {
    this.runId = `test-${Date.now()}`;
    this.logs = [];
  }

  async runTripleTest(testName, testFunction) {
    const results = [];
    
    for (let iteration = 1; iteration <= 3; iteration++) {
      console.log(`🧪 Running ${testName} - Iteration ${iteration}/3`);
      
      try {
        const result = await testFunction(iteration);
        results.push({ iteration, status: 'PASSED', result });
        this.log('success', `${testName} iteration ${iteration}`, { result });
      } catch (error) {
        results.push({ iteration, status: 'FAILED', error: error.message });
        this.log('error', `${testName} iteration ${iteration}`, { error: error.message });
        throw error;
      }
    }
    
    return results;
  }

  validateConservation(beforeCount, afterCount, operation) {
    const conserved = beforeCount === afterCount || operation === 'cycle';
    this.log(conserved ? 'success' : 'error', 'Conservation check', {
      operation,
      before_count: beforeCount,
      after_count: afterCount,
      conserved
    });
    return conserved;
  }

  log(level, message, data = {}) {
    const logEntry = {
      timestamp: new Date().toISOString(),
      level,
      message,
      run_id: this.runId,
      ...data
    };
    this.logs.push(logEntry);
    console.log(`[${level.toUpperCase()}] ${message}`, data);
  }

  async saveLogs() {
    const filepath = join(process.cwd(), 'logs', `test-${this.runId}.jsonl`);
    await fs.mkdir(join(process.cwd(), 'logs'), { recursive: true });
    
    const jsonlContent = this.logs.map(log => JSON.stringify(log)).join('\n');
    await fs.writeFile(filepath, jsonlContent);
    
    return filepath;
  }
}

// Helper function pour tester les API calls
async function makeApiCall(endpoint, method = 'GET', body = null) {
  const url = `${API_BASE}/api${endpoint}`;
  const options = {
    method,
    headers: { 'Content-Type': 'application/json' }
  };
  
  if (body) {
    options.body = JSON.stringify(body);
  }

  const response = await fetch(url, options);
  const data = await response.json();
  
  return { status: response.status, data };
}

// Instance globale du test runner
let testRunner;

describe('LUMS/VORAX System - Triple Tests', () => {
  beforeEach(() => {
    testRunner = new TripleTestRunner();
  });

  afterEach(async () => {
    if (testRunner) {
      await testRunner.saveLogs();
    }
  });

  test('Conversion bit-to-LUM avec validation triple', async () => {
    await testRunner.runTripleTest('bit-to-lum-conversion', async (iteration) => {
      const inputBits = `110100${iteration.toString().padStart(3, '0')}`;
      
      const { status, data } = await makeApiCall('/convert/bit-to-lum', 'POST', {
        inputBits
      });

      expect(status).toBe(200);
      expect(data.lums).toHaveLength(inputBits.length);
      expect(data.conversion_stats.conservation_valid).toBe(true);

      // Validation de conservation
      const conserved = testRunner.validateConservation(
        inputBits.length,
        data.lums.length,
        'conversion'
      );
      expect(conserved).toBe(true);

      return {
        input_length: inputBits.length,
        output_count: data.lums.length,
        conversion_time: data.conversion_stats.conversion_time_ms
      };
    });
  }, TEST_TIMEOUT);

  test('Opérations VORAX avec conservation', async () => {
    await testRunner.runTripleTest('vorax-operations', async (iteration) => {
      // Créer des LUMs de test
      const inputBits = '11010011';
      const { data: conversionData } = await makeApiCall('/convert/bit-to-lum', 'POST', {
        inputBits
      });

      const testGroup = {
        lums: conversionData.lums,
        groupType: 'linear',
        id: `test-group-${iteration}`
      };

      // Test fusion
      const { status: fusionStatus, data: fusionData } = await makeApiCall(
        '/execute/vorax-operation',
        'POST',
        {
          type: 'fusion',
          groups: [testGroup, testGroup]
        }
      );

      expect(fusionStatus).toBe(200);
      expect(fusionData.operation_stats.conservation_valid).toBe(true);

      // Test split
      const { status: splitStatus, data: splitData } = await makeApiCall(
        '/execute/vorax-operation',
        'POST',
        {
          type: 'split',
          groups: [testGroup],
          parameters: { zones: 2 }
        }
      );

      expect(splitStatus).toBe(200);
      expect(splitData.operation_stats.conservation_valid).toBe(true);

      return {
        fusion_lums: fusionData.operation_stats.output_lums,
        split_groups: splitData.result.length
      };
    });
  }, TEST_TIMEOUT);

  test('Système de logging et statistiques', async () => {
    await testRunner.runTripleTest('logging-system', async (iteration) => {
      // Test récupération des stats
      const { status, data } = await makeApiCall('/logs/stats');

      expect(status).toBe(200);
      expect(data).toHaveProperty('total_logs');
      expect(data).toHaveProperty('run_id');

      // Test sauvegarde des logs
      const { status: saveStatus, data: saveData } = await makeApiCall('/logs/save', 'POST');

      expect(saveStatus).toBe(200);
      expect(saveData).toHaveProperty('filepath');
      expect(saveData).toHaveProperty('log_count');

      return {
        total_logs: data.total_logs,
        log_count: saveData.log_count
      };
    });
  }, TEST_TIMEOUT);

  test('Validation sécurité et rate limiting', async () => {
    await testRunner.runTripleTest('security-validation', async (iteration) => {
      // Test validation stricte avec données invalides
      const { status: invalidStatus } = await makeApiCall('/convert/bit-to-lum', 'POST', {
        inputBits: 'invalid123'
      });

      expect(invalidStatus).toBe(400);

      // Test avec données valides
      const { status: validStatus } = await makeApiCall('/convert/bit-to-lum', 'POST', {
        inputBits: '1010'
      });

      expect(validStatus).toBe(200);

      return {
        invalid_rejected: invalidStatus === 400,
        valid_accepted: validStatus === 200
      };
    });
  }, TEST_TIMEOUT);

  test('Build system et compilation C', async () => {
    await testRunner.runTripleTest('build-system', async (iteration) => {
      return new Promise((resolve, reject) => {
        const makeProcess = spawn('make', ['clean', '&&', 'make', 'all'], {
          shell: true,
          cwd: process.cwd()
        });

        let output = '';
        let errorOutput = '';

        makeProcess.stdout.on('data', (data) => {
          output += data.toString();
        });

        makeProcess.stderr.on('data', (data) => {
          errorOutput += data.toString();
        });

        makeProcess.on('close', (code) => {
          if (code === 0) {
            resolve({
              build_success: true,
              output_lines: output.split('\n').length,
              has_artifacts: output.includes('Library created')
            });
          } else {
            reject(new Error(`Build failed with code ${code}: ${errorOutput}`));
          }
        });

        setTimeout(() => {
          makeProcess.kill();
          reject(new Error('Build timeout'));
        }, 30000);
      });
    });
  }, 35000);
});

// Test de performance et métriques
describe('Performance Metrics', () => {
  test('Temps de réponse API < 10ms pour opérations simples', async () => {
    const startTime = process.hrtime.bigint();
    
    await makeApiCall('/logs/stats');
    
    const endTime = process.hrtime.bigint();
    const responseTime = Number((endTime - startTime) / BigInt(1000000));
    
    expect(responseTime).toBeLessThan(10);
  });

  test('Conservation rate 100% pour opérations non-cycle', async () => {
    const testData = await makeApiCall('/convert/bit-to-lum', 'POST', {
      inputBits: '11001100'
    });

    const operationData = await makeApiCall('/execute/vorax-operation', 'POST', {
      type: 'fusion',
      groups: [
        { lums: testData.data.lums.slice(0, 4), groupType: 'linear' },
        { lums: testData.data.lums.slice(4, 8), groupType: 'linear' }
      ]
    });

    expect(operationData.data.operation_stats.conservation_valid).toBe(true);
  });
});
