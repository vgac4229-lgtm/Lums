import type { Express } from "express";
import { createServer, type Server } from "http";
import { storage } from "./storage";
import { insertLumSessionSchema, insertLumOperationSchema } from "@shared/schema";
import { z } from "zod";

export async function registerRoutes(app: Express): Promise<Server> {
  // LUM Sessions routes
  app.get("/api/sessions", async (req, res) => {
    try {
      const sessions = await storage.getAllLumSessions();
      res.json(sessions);
    } catch (error) {
      res.status(500).json({ message: "Failed to fetch sessions" });
    }
  });

  app.get("/api/sessions/:id", async (req, res) => {
    try {
      const session = await storage.getLumSession(req.params.id);
      if (!session) {
        return res.status(404).json({ message: "Session not found" });
      }
      res.json(session);
    } catch (error) {
      res.status(500).json({ message: "Failed to fetch session" });
    }
  });

  app.post("/api/sessions", async (req, res) => {
    try {
      const sessionData = insertLumSessionSchema.parse(req.body);
      const session = await storage.createLumSession(sessionData);
      res.status(201).json(session);
    } catch (error) {
      if (error instanceof z.ZodError) {
        return res.status(400).json({ message: "Invalid session data", errors: error.errors });
      }
      res.status(500).json({ message: "Failed to create session" });
    }
  });

  app.put("/api/sessions/:id", async (req, res) => {
    try {
      const sessionData = insertLumSessionSchema.partial().parse(req.body);
      const session = await storage.updateLumSession(req.params.id, sessionData);
      if (!session) {
        return res.status(404).json({ message: "Session not found" });
      }
      res.json(session);
    } catch (error) {
      if (error instanceof z.ZodError) {
        return res.status(400).json({ message: "Invalid session data", errors: error.errors });
      }
      res.status(500).json({ message: "Failed to update session" });
    }
  });

  app.delete("/api/sessions/:id", async (req, res) => {
    try {
      const deleted = await storage.deleteLumSession(req.params.id);
      if (!deleted) {
        return res.status(404).json({ message: "Session not found" });
      }
      res.status(204).send();
    } catch (error) {
      res.status(500).json({ message: "Failed to delete session" });
    }
  });

  // LUM Operations routes
  app.get("/api/sessions/:sessionId/operations", async (req, res) => {
    try {
      const operations = await storage.getOperationsBySession(req.params.sessionId);
      res.json(operations);
    } catch (error) {
      res.status(500).json({ message: "Failed to fetch operations" });
    }
  });

  app.post("/api/operations", async (req, res) => {
    try {
      const operationData = insertLumOperationSchema.parse(req.body);
      const operation = await storage.createLumOperation(operationData);
      res.status(201).json(operation);
    } catch (error) {
      if (error instanceof z.ZodError) {
        return res.status(400).json({ message: "Invalid operation data", errors: error.errors });
      }
      res.status(500).json({ message: "Failed to create operation" });
    }
  });

  // Bit to LUM conversion endpoint
  app.post("/api/convert/bit-to-lum", async (req, res) => {
    try {
      const { bits } = req.body;
      if (typeof bits !== 'string' || !/^[01]+$/.test(bits)) {
        return res.status(400).json({ message: "Invalid binary string" });
      }

      // Convert bits to LUMS representation
      const lums = bits.split('').map((bit, index) => ({
        presence: parseInt(bit),
        structureType: 'linear' as const,
        position: { x: index * 20, y: 0 },
      }));

      res.json({ inputBits: bits, outputLums: lums });
    } catch (error) {
      res.status(500).json({ message: "Failed to convert bits to LUMS" });
    }
  });

  // VORAX operation execution endpoint
  app.post("/api/execute/vorax-operation", async (req, res) => {
    try {
      const { operation, inputGroups, parameters } = req.body;
      
      // Execute VORAX operation based on type
      let result;
      switch (operation) {
        case 'fusion':
          result = executeVoraxFusion(inputGroups, parameters);
          break;
        case 'split':
          result = executeVoraxSplit(inputGroups, parameters);
          break;
        case 'cycle':
          result = executeVoraxCycle(inputGroups, parameters);
          break;
        case 'flow':
          result = executeVoraxFlow(inputGroups, parameters);
          break;
        default:
          return res.status(400).json({ message: "Unknown operation type" });
      }

      res.json({ operation, result });
    } catch (error) {
      res.status(500).json({ message: "Failed to execute VORAX operation" });
    }
  });

  // Generate C code endpoint
  app.post("/api/generate/c-code", async (req, res) => {
    try {
      const { codeType, parameters } = req.body;
      
      let generatedCode = "";
      switch (codeType) {
        case 'encoder':
          generatedCode = generateEncoderCode(parameters);
          break;
        case 'decoder':
          generatedCode = generateDecoderCode(parameters);
          break;
        case 'operations':
          generatedCode = generateOperationsCode(parameters);
          break;
        default:
          return res.status(400).json({ message: "Unknown code type" });
      }

      res.json({ codeType, generatedCode });
    } catch (error) {
      res.status(500).json({ message: "Failed to generate C code" });
    }
  });

  const httpServer = createServer(app);
  return httpServer;
}

// VORAX operation implementations
function executeVoraxFusion(inputGroups: any[], parameters: any) {
  // Combine all LUMS from input groups
  const allLums = inputGroups.flat();
  return {
    id: `fusion-${Date.now()}`,
    lums: allLums,
    groupType: 'cluster',
  };
}

function executeVoraxSplit(inputGroups: any[], parameters: any) {
  const { zones = 2 } = parameters || {};
  const allLums = inputGroups.flat();
  const lumsPerZone = Math.floor(allLums.length / zones);
  
  const result = [];
  for (let i = 0; i < zones; i++) {
    const startIndex = i * lumsPerZone;
    const endIndex = i === zones - 1 ? allLums.length : (i + 1) * lumsPerZone;
    result.push({
      id: `split-${i}-${Date.now()}`,
      lums: allLums.slice(startIndex, endIndex),
      groupType: 'linear',
    });
  }
  
  return result;
}

function executeVoraxCycle(inputGroups: any[], parameters: any) {
  const { modulo = 3 } = parameters || {};
  return inputGroups.map(group => ({
    ...group,
    lums: group.lums.slice(0, group.lums.length % modulo),
    id: `cycle-${Date.now()}`,
  }));
}

function executeVoraxFlow(inputGroups: any[], parameters: any) {
  const { targetZone = 'ZoneB' } = parameters || {};
  return inputGroups.map(group => ({
    ...group,
    id: `flow-${Date.now()}`,
    targetZone,
  }));
}

// C code generation functions
function generateEncoderCode(parameters: any) {
  const { dataType = 'uint8_t', outputMode = 'struct' } = parameters || {};
  
  return `#include <stdio.h>
#include <stdlib.h>
#include "lums.h"

LUM* encode_bit_to_lum(${dataType} input) {
    int bit_count = sizeof(${dataType}) * 8;
    LUM* result = malloc(sizeof(LUM) * bit_count);
    
    for (int i = 0; i < bit_count; i++) {
        result[i].presence = (input >> i) & 1;
        result[i].structure_type = LUM_LINEAR;
        result[i].spatial_data = NULL;
    }
    
    return result;
}

LUMGroup* encode_to_lum_group(${dataType} input) {
    LUMGroup* group = malloc(sizeof(LUMGroup));
    group->lums = encode_bit_to_lum(input);
    group->count = sizeof(${dataType}) * 8;
    group->group_type = GROUP_LINEAR;
    
    return group;
}`;
}

function generateDecoderCode(parameters: any) {
  const { dataType = 'uint8_t' } = parameters || {};
  
  return `#include <stdio.h>
#include "lums.h"

${dataType} decode_lum_to_bit(LUM* lums, int count) {
    ${dataType} result = 0;
    
    for (int i = 0; i < count && i < sizeof(${dataType}) * 8; i++) {
        if (lums[i].presence) {
            result |= (1 << i);
        }
    }
    
    return result;
}

${dataType} decode_lum_group(LUMGroup* group) {
    return decode_lum_to_bit(group->lums, group->count);
}`;
}

function generateOperationsCode(parameters: any) {
  return `#include <stdio.h>
#include <stdlib.h>
#include "lums.h"

// Opération de Fusion (⧉)
LUMGroup* lum_fusion(LUMGroup* group1, LUMGroup* group2) {
    LUMGroup* result = malloc(sizeof(LUMGroup));
    result->count = group1->count + group2->count;
    result->lums = malloc(sizeof(LUM) * result->count);
    result->group_type = GROUP_CLUSTER;
    
    // Copy group1 lums
    for (int i = 0; i < group1->count; i++) {
        result->lums[i] = group1->lums[i];
    }
    
    // Copy group2 lums
    for (int i = 0; i < group2->count; i++) {
        result->lums[group1->count + i] = group2->lums[i];
    }
    
    return result;
}

// Opération de Répartition (⇅)
LUMGroup** lum_split(LUMGroup* source, int zones) {
    LUMGroup** result = malloc(sizeof(LUMGroup*) * zones);
    int lums_per_zone = source->count / zones;
    int remainder = source->count % zones;
    
    int current_index = 0;
    for (int i = 0; i < zones; i++) {
        result[i] = malloc(sizeof(LUMGroup));
        int zone_count = lums_per_zone + (i < remainder ? 1 : 0);
        result[i]->count = zone_count;
        result[i]->lums = malloc(sizeof(LUM) * zone_count);
        result[i]->group_type = GROUP_LINEAR;
        
        for (int j = 0; j < zone_count; j++) {
            result[i]->lums[j] = source->lums[current_index++];
        }
    }
    
    return result;
}

// Opération de Cycle (⟲)
LUMGroup* lum_cycle(LUMGroup* source, int modulo) {
    LUMGroup* result = malloc(sizeof(LUMGroup));
    result->count = source->count % modulo;
    result->lums = malloc(sizeof(LUM) * result->count);
    result->group_type = GROUP_CYCLE;
    
    for (int i = 0; i < result->count; i++) {
        result->lums[i] = source->lums[i];
    }
    
    return result;
}`;
}
