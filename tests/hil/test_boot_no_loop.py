import re
import sys
import time


def test_boot_no_loop(dut):
    """Verify that the DUT boots once and does not fall into a reset loop."""
    sys.stdout.write("[hosted-safe] Monitoring boot log for resets...\n")
    dut.expect_exact("Multicore app", timeout=15)
    dut.expect(re.compile(r"Project name:.*m5tab5_userdemo"), timeout=10)
    time.sleep(8)
    dut._expect_text_not_exist("Guru Meditation Error", timeout=1)
    dut._expect_text_not_exist("rst:0xc (SW_CPU_RESET)", timeout=1)
