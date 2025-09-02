
#!/bin/bash

# Génération logs forensiques avec timestamp fixe
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
LOG_DIR="logs/inspection_critique_${TIMESTAMP}"
EXECUTION_LOG="${LOG_DIR}/execution_complete.log"

echo "=== SCRIPT GÉNÉRATION LOGS FORENSIQUES CORRIGÉ ==="
echo "Timestamp session: ${TIMESTAMP}"
echo "Répertoire cible: ${LOG_DIR}"

# Création structure répertoires
echo "Création structure répertoires..."
mkdir -p "${LOG_DIR}"
mkdir -p "${LOG_DIR}/performance"
mkdir -p "${LOG_DIR}/validation"
mkdir -p "${LOG_DIR}/security"
mkdir -p "${LOG_DIR}/forensic"

# Vérification création réussie
if [ ! -d "${LOG_DIR}" ]; then
    echo "❌ ERREUR: Impossible de créer ${LOG_DIR}"
    exit 1
fi

echo "✅ Structure répertoires créée avec succès"

# Génération header logs
echo "=== LOGS FORENSIQUES GÉNÉRATION ===" > "${EXECUTION_LOG}"
echo "Date: $(date -u '+%Y-%m-%d %H:%M:%S UTC')" >> "${EXECUTION_LOG}"
echo "Session: ${TIMESTAMP}" >> "${EXECUTION_LOG}"
echo "Système: $(uname -a)" >> "${EXECUTION_LOG}"
echo "=====================================" >> "${EXECUTION_LOG}"
echo "" >> "${EXECUTION_LOG}"

# Copie logs scientifiques existants
echo "=== COPIE LOGS SCIENTIFIQUES EXISTANTS ===" >> "${EXECUTION_LOG}"
if [ -f "logs/scientific_traces/lums_operations.jsonl" ]; then
    cp "logs/scientific_traces/lums_operations.jsonl" "${LOG_DIR}/forensic/lums_operations_backup.jsonl"
    echo "✅ Sauvegarde lums_operations.jsonl créée" >> "${EXECUTION_LOG}"
    
    # Statistiques logs LUMS
    TOTAL_OPERATIONS=$(wc -l < "logs/scientific_traces/lums_operations.jsonl")
    echo "📊 Total opérations LUMS: ${TOTAL_OPERATIONS}" >> "${EXECUTION_LOG}"
    
    # Dernières 10 opérations
    echo "=== DERNIÈRES 10 OPÉRATIONS LUMS ===" >> "${EXECUTION_LOG}"
    tail -10 "logs/scientific_traces/lums_operations.jsonl" >> "${EXECUTION_LOG}"
else
    echo "⚠️ Aucun log lums_operations.jsonl trouvé" >> "${EXECUTION_LOG}"
fi

# Listing complet structure logs
echo "" >> "${EXECUTION_LOG}"
echo "=== STRUCTURE COMPLÈTE LOGS ===" >> "${EXECUTION_LOG}"
find logs/ -type f -name "*.log" -o -name "*.jsonl" -o -name "*.json" | sort >> "${EXECUTION_LOG}"

# État compilation système
echo "" >> "${EXECUTION_LOG}"
echo "=== ÉTAT COMPILATION SYSTÈME ===" >> "${EXECUTION_LOG}"
if [ -f "build/liblums.a" ]; then
    echo "✅ liblums.a présent: $(ls -lh build/liblums.a)" >> "${EXECUTION_LOG}"
else
    echo "❌ liblums.a manquant" >> "${EXECUTION_LOG}"
fi

if [ -f "build/electromechanical_console" ]; then
    echo "✅ electromechanical_console présent: $(ls -lh build/electromechanical_console)" >> "${EXECUTION_LOG}"
else
    echo "❌ electromechanical_console manquant" >> "${EXECUTION_LOG}"
fi

# Génération métriques système
echo "" >> "${EXECUTION_LOG}"
echo "=== MÉTRIQUES SYSTÈME TEMPS RÉEL ===" >> "${EXECUTION_LOG}"
echo "CPU Usage: $(top -bn1 | grep "Cpu(s)" | head -1)" >> "${EXECUTION_LOG}"
echo "Memory: $(free -h | grep Mem)" >> "${EXECUTION_LOG}"
echo "Disk: $(df -h . | tail -1)" >> "${EXECUTION_LOG}"

# Hash forensique du log pour intégrité
LOG_HASH=$(sha256sum "${EXECUTION_LOG}" | cut -d' ' -f1)
echo "" >> "${EXECUTION_LOG}"
echo "=== INTÉGRITÉ FORENSIQUE ===" >> "${EXECUTION_LOG}"
echo "Hash SHA256: ${LOG_HASH}" >> "${EXECUTION_LOG}"
echo "Taille: $(stat -c%s "${EXECUTION_LOG}") bytes" >> "${EXECUTION_LOG}"
echo "Généré: $(date -u '+%Y-%m-%d %H:%M:%S.%N UTC')" >> "${EXECUTION_LOG}"

echo ""
echo "✅ LOGS FORENSIQUES GÉNÉRÉS AVEC SUCCÈS"
echo "📁 Répertoire: ${LOG_DIR}"
echo "📄 Log principal: ${EXECUTION_LOG}"
echo "🔒 Hash intégrité: ${LOG_HASH}"
echo ""
echo "Pour visualiser:"
echo "  cat ${EXECUTION_LOG}"
echo "  ls -la ${LOG_DIR}/"
