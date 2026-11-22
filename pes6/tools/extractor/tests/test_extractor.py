import hashlib
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

EXTRACTOR = Path(__file__).resolve().parents[1] / "pes6-extractor"
RUNNER = ["qemu-aarch64-static", str(EXTRACTOR)]
ISO = os.environ.get("PES6_TEST_ISO")
EXPECTED = "c3975f60d20a2510788c2441f23e212e0d7eb688b744f33ca330edd1ef19d9ab"


class ExtractorTests(unittest.TestCase):
    def run_extractor(self, iso, output):
        return subprocess.run([*RUNNER, str(iso), str(output)], capture_output=True, text=True)

    def test_invalid_iso_fails_without_creating_output(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            iso = root / "invalid.iso"
            iso.write_bytes(b"not an iso")
            result = self.run_extractor(iso, root / "EBOOT.elf")
            self.assertNotEqual(result.returncode, 0)
            self.assertFalse((root / "EBOOT.elf").exists())

    def test_missing_iso_reports_failure(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            result = self.run_extractor(root / "missing.iso", root / "EBOOT.elf")
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("ISO", result.stderr)
            self.assertFalse((root / "EBOOT.elf").exists())

    def test_wrong_iso_preserves_existing_executable(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            iso = root / "wrong.iso"
            iso.write_bytes(b"wrong game")
            output = root / "EBOOT.elf"
            output.write_bytes(b"user data")
            result = self.run_extractor(iso, output)
            self.assertNotEqual(result.returncode, 0)
            self.assertEqual(output.read_bytes(), b"user data")

    @unittest.skipUnless(ISO, "PES6_TEST_ISO is required")
    def test_real_iso_extracts_and_reuses_expected_executable(self):
        with tempfile.TemporaryDirectory() as folder:
            output = Path(folder) / "EBOOT.elf"
            first = self.run_extractor(ISO, output)
            self.assertEqual(first.returncode, 0, first.stderr)
            self.assertEqual(hashlib.sha256(output.read_bytes()).hexdigest(), EXPECTED)
            before = output.stat().st_mtime_ns
            second = self.run_extractor(ISO, output)
            self.assertEqual(second.returncode, 0, second.stderr)
            self.assertEqual(output.stat().st_mtime_ns, before)

    @unittest.skipUnless(ISO, "PES6_TEST_ISO is required")
    def test_invalid_existing_elf_is_preserved(self):
        with tempfile.TemporaryDirectory() as folder:
            output = Path(folder) / "EBOOT.elf"
            output.write_bytes(b"user data")
            result = self.run_extractor(ISO, output)
            self.assertNotEqual(result.returncode, 0)
            self.assertEqual(output.read_bytes(), b"user data")


if __name__ == "__main__":
    unittest.main()
