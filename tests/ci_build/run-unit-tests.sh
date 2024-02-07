#!/usr/bin/env bash
set -eux -o pipefail
PROJECT_DIR="$(readlink -f "$(dirname "${BASH_SOURCE[0]}")/../..")"
WORKSPACE=${WORKSPACE:-"${PROJECT_DIR}"}
die() { echo "Error: $*" >&2; exit 1; }

test_filter="${1:-}"
[ -n "${test_filter}" ] || test_filter="*"
echo "test_filter: ${test_filter}"

export SAUNAFS_ROOT=${WORKSPACE}/install/saunafs
echo "SAUNAFS_ROOT: ${SAUNAFS_ROOT}"
export TEST_OUTPUT_DIR=${WORKSPACE}/test_output
echo "TEST_OUTPUT_DIR: ${TEST_OUTPUT_DIR}"
export TERM=xterm

killall -9 saunafs-tests || true
mkdir -m 777 -p "${TEST_OUTPUT_DIR}"
rm -rf "${TEST_OUTPUT_DIR:?}"/{,.}* || true
rm -rf /mnt/ramdisk/{,.}*  || true
[ -f "${WORKSPACE}/build/saunafs/src/unittests/unittests" ] || \
	die "${WORKSPACE}/build/saunafs/src/unittests/unittests" not found, did you build the project?
"${WORKSPACE}/build/saunafs/src/unittests/unittests" --gtest_color=yes --gtest_filter="${test_filter}" --gtest_output=xml:"${TEST_OUTPUT_DIR}/unit_test_results.xml"
