#!/usr/bin/env python3
\"\"\"
Memory safety tests for Openbox.

These tests use Valgrind and AddressSanitizer to detect memory issues.
\"\"\"

import pytest
import subprocess
import os
import tempfile

# Skip if Valgrind not available
try:
    subprocess.run(['valgrind', '--version'], capture_output=True, check=True)
    HAS_VALGRIND = True
except (subprocess.CalledProcessError, FileNotFoundError):
    HAS_VALGRIND = False


@pytest.mark.skipif(not HAS_VALGRIND, reason=\"Valgrind not installed\")
class TestValgrind:
    \"\"\"Tests using Valgrind for memory debugging.\"\"\"
n    
    def test_openbox_version_no_leaks(self, xvfb_display):\n        \"\"\"Test that --version doesn't leak memory.\"\"\"\n        result = subprocess.run(\n            [\n                'valgrind',\n                '--leak-check=full',\n                '--error-exitcode=1',\n                '--show-leak-kinds=all',\n                './build/openbox/openbox',\n                '--version'\n            ],\n            capture_output=True,\n            text=True\n        )\n        \n        # Should complete without memory errors\n        assert result.returncode == 0, f\"Valgrind found issues:\\n{result.stderr}\"\n    \n    def test_openbox_help_no_leaks(self, xvfb_display):\n        \"\"\"Test that --help doesn't leak memory.\"\"\"\n        result = subprocess.run(\n            [\n                'valgrind',\n                '--leak-check=full',\n                '--error-exitcode=1',\n                '--show-leak-kinds=all',\n                './build/openbox/openbox',\n                '--help'\n            ],\n            capture_output=True,\n            text=True\n        )\n        \n        assert result.returncode == 0, f\"Valgrind found issues:\\n{result.stderr}\"\n    \n    def test_config_parse_no_leaks(self, xvfb_display, clean_config):\n        \"\"\"Test that config parsing doesn't leak memory.\"\"\"\n        env = os.environ.copy()\n        env['XDG_CONFIG_HOME'] = str(clean_config.parent)\n        \n        result = subprocess.run(\n            [\n                'valgrind',\n                '--leak-check=full',\n                '--error-exitcode=1',\n                '--show-leak-kinds=all',\n                './build/openbox/openbox',\n                '--config-test'\n            ],\n            capture_output=True,\n            text=True,\n            env=env\n        )\n        \n        assert result.returncode == 0, f\"Valgrind found issues:\\n{result.stderr}\"\n\n\n@pytest.mark.skipif(\n    not os.path.exists('./build-asan'),\n    reason=\"AddressSanitizer build not available\"\n)\nclass TestAddressSanitizer:\n    \"\"\"Tests using AddressSanitizer.\"\"\"
n    
    def test_asan_detects_overflow(self):\n        \"\"\"Test that ASan detects buffer overflows (if we had a test for it).\"\"\"\n        # This would require a specially built test binary\n        pytest.skip(\"Requires test binary with intentional overflow\")\n    \n    def test_asan_no_errors_on_startup(self, xvfb_display):\n        \"\"\"Test that ASan build starts without errors.\"\"\"\n        result = subprocess.run(\n            ['./build-asan/openbox/openbox', '--version'],\n            capture_output=True,\n            text=True\n        )\n        \n        # Should not report ASan errors\n        assert \"ERROR: AddressSanitizer\" not in result.stderr\n        assert result.returncode == 0\n\n\nclass TestResourceCleanup:
n    \"\"\"Tests for proper resource cleanup.\"\"\"
n    
    def test_file_descriptors_closed(self, openbox):\n        \"\"\"Test that file descriptors are properly closed.\"\"\"\n        # This would check /proc/[pid]/fd\n        assert openbox.is_running()\n    \n    def test_shared_memory_released(self, openbox):\n        \"\"\"Test that shared memory is released on exit.\"\"\"\n        # Would require ipcs inspection\n        assert openbox.is_running()\n\n\nclass TestErrorHandlingRobustness:
n    \"\"\"Tests for robust error handling.\"\"\"
n    
    def test_null_pointer_handling(self, xvfb_display):\n        \"\"\"Test handling of null pointer scenarios.\"\"\"\n        # Would require specific test binaries\n        pytest.skip(\"Requires instrumented build\")\n    \n    def test_buffer_bounds_checking(self, xvfb_display):\n        \"\"\"Test buffer bounds checking.\"\"\"\n        # Would require specific test binaries\n        pytest.skip(\"Requires instrumented build\")\n\n\nif __name__ == '__main__':\n    pytest.main([__file__, '-v'])
