#!/bin/bash
set -euo pipefail

#Scripts directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
#Project directory 
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

#Build directory
RayTracer="$PROJECT_ROOT/bin/RayTracer"

echo "Running: $RayTracer" 

"$RayTracer"