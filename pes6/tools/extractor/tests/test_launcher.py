import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[4]


class LauncherTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.game = self.root / "pes6"
        self.game.mkdir()
        shutil.copy(ROOT / "PES 6.sh", self.root / "PES 6.sh")
        fake_bin = self.root / "bin"
        fake_bin.mkdir()
        sudo = fake_bin / "sudo"
        sudo.write_text("#!/bin/sh\nexit 0\n")
        sudo.chmod(0o755)
        self.env = dict(os.environ, PATH=f"{fake_bin}:{os.environ['PATH']}")

    def launch(self):
        return subprocess.run(["bash", str(self.root / "PES 6.sh")], env=self.env)

    def test_extracts_from_gamedata_to_assets(self):
        data = self.game / "gamedata"
        data.mkdir()
        iso = data / "my-game.iso"
        iso.write_bytes(b"fixture")
        tools = self.game / "tools"
        tools.mkdir()
        extractor_dir = tools / "extractor"
        extractor_dir.mkdir()
        extractor = extractor_dir / "pes6-extractor"
        extractor.write_text('#!/bin/sh\nprintf "%s\\n" "$1" "$2" > "$3"\nexit 23\n'.replace('"$3"', '"$RECORD"'))
        extractor.chmod(0o755)
        self.env["RECORD"] = str(self.root / "arguments")
        result = self.launch()
        self.assertEqual(result.returncode, 14)
        self.assertEqual((self.root / "arguments").read_text().splitlines(), [str(iso), str(data / "assets" / "EBOOT.elf")])
        self.assertTrue((data / "assets").is_dir())
        self.assertEqual(iso.read_bytes(), b"fixture")

    def test_missing_gamedata_iso_reports_error(self):
        (self.game / "pes6.iso").write_bytes(b"old location")
        result = self.launch()
        self.assertEqual(result.returncode, 12)
        self.assertIn("pes6/gamedata/", (self.game / "log.txt").read_text())


if __name__ == "__main__":
    unittest.main()
