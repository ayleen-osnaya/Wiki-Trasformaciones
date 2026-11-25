#!/usr/bin/env python3
"""Interfaz (PySide6) para transformadas.c. No modifica el codigo en C.

Copia las imagenes elegidas a imagenes/Imagen1..N.bmp, ejecuta transformadas
(lo compila si falta) y muestra resultados/ImagenK/. Al final restaura imagenes/.
"""
import os
import shutil
import struct
import subprocess
import sys
from pathlib import Path

from PySide6.QtCore import QByteArray, QProcess, QSize, Qt, QTimer, QUrl
from PySide6.QtGui import QDesktopServices, QIcon, QPixmap
from PySide6.QtWidgets import *

HERE = Path(__file__).resolve().parent
WORK = HERE  # imagenes/ y resultados/ viven junto a transformadas.c
EXE = HERE / ("transformadas.exe" if os.name == "nt" else "transformadas")
SOURCE = HERE / "transformadas.c"
MAX_IMAGES = 10  # TOTAL_IMAGENES en transformadas.c

# (codigo, etiqueta, archivo que deja transformadas.c en resultados/ImagenK/)
TRANSFORMS = [
    ("ivg", "Inversión vertical gris", "inv_ver_bn.bmp"),
    ("ivc", "Inversión vertical color", "inv_ver_color.bmp"),
    ("ihg", "Espejo horizontal gris", "inv_hor_bn.bmp"),
    ("ihc", "Espejo horizontal color", "inv_hor_color.bmp"),
    ("bg", "Desenfoque gris", "blur_bn.bmp"),
    ("bc", "Desenfoque color", "blur_color.bmp"),
]


def bmp_size(path):
    """(ancho, alto) si es BMP de 24 bits sin compresion; si no, None."""
    try:
        with open(path, "rb") as f:
            h = f.read(54)
        w, hh = struct.unpack_from("<ii", h, 18)
        bpp, = struct.unpack_from("<H", h, 28)
        comp, = struct.unpack_from("<I", h, 30)
    except (OSError, struct.error):
        return None
    return (w, abs(hh)) if h[:2] == b"BM" and bpp == 24 and comp == 0 and w > 0 else None


class DropList(QListWidget):
    """Lista que acepta archivos .bmp arrastrados desde el explorador."""

    def __init__(self, on_drop):
        super().__init__()
        self.on_drop = on_drop
        self.setAcceptDrops(True)
        self.setStyleSheet("QListWidget { background: #eaf6f0; border: 2px dashed #5a9a88; }")

    def _paths(self, e):
        return [u.toLocalFile() for u in e.mimeData().urls() if u.toLocalFile().lower().endswith(".bmp")]

    def dragEnterEvent(self, e):
        e.acceptProposedAction() if self._paths(e) else e.ignore()

    dragMoveEvent = dragEnterEvent

    def dropEvent(self, e):
        e.acceptProposedAction()
        self.on_drop(self._paths(e))


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("BMP Parallel Studio")
        self.resize(1100, 760)
        self.files, self.selected, self.proc, self.out = [], [], None, []
        self.buf = QByteArray()
        self.timer = QTimer(self, interval=250)
        self.timer.timeout.connect(self.poll)
        central = QWidget()
        self.setCentralWidget(central)
        root = QHBoxLayout(central)

        # ---- izquierda: imagenes + registro/resultados
        left = QVBoxLayout()
        root.addLayout(left, 3)
        left.addWidget(QLabel(f"<b>Imágenes BMP</b> — arrastra hasta {MAX_IMAGES} archivos aquí o usa Agregar BMP"))
        self.list = DropList(self.add_files)
        self.list.setViewMode(QListWidget.ViewMode.IconMode)
        self.list.setIconSize(QSize(100, 100))
        self.list.setGridSize(QSize(130, 140))
        self.list.setResizeMode(QListWidget.ResizeMode.Adjust)
        self.list.setMovement(QListWidget.Movement.Static)
        left.addWidget(self.list, 2)
        row = QHBoxLayout()
        for text, fn in (("Agregar BMP", self.pick), ("Limpiar", self.clear)):
            btn = QPushButton(text)
            btn.clicked.connect(fn)
            row.addWidget(btn)
        self.count = QLabel(f"0 / {MAX_IMAGES}")
        row.addStretch()
        row.addWidget(self.count)
        left.addLayout(row)

        self.tabs = QTabWidget()
        self.log = QPlainTextEdit(readOnly=True)
        self.tabs.addTab(self.log, "Registro")
        res = QWidget()
        lay = QVBoxLayout(res)
        top = QHBoxLayout()
        self.summary = QLabel("Aún no hay resultados.")
        self.combo = QComboBox()
        self.combo.currentIndexChanged.connect(self.show_result)
        open_btn = QPushButton("Abrir carpeta")
        open_btn.clicked.connect(lambda: QDesktopServices.openUrl(QUrl.fromLocalFile(str(WORK / "resultados"))))
        for w, stretch in ((self.summary, 1), (self.combo, 0), (open_btn, 0)):
            top.addWidget(w, stretch)
        lay.addLayout(top)
        scroll = QScrollArea()
        scroll.setWidgetResizable(True)
        host = QWidget()
        self.gallery = QGridLayout(host)
        self.gallery.setAlignment(Qt.AlignmentFlag.AlignTop | Qt.AlignmentFlag.AlignLeft)
        scroll.setWidget(host)
        lay.addWidget(scroll, 1)
        self.tabs.addTab(res, "Resultados")
        left.addWidget(self.tabs, 3)

        # ---- derecha: opciones
        right = QVBoxLayout()
        root.addLayout(right, 1)
        right.addWidget(QLabel("<b>Opciones de procesamiento</b>"))
        box = QGroupBox("Transformaciones")
        grid = QGridLayout(box)
        self.all = QCheckBox("Todas")
        self.all.clicked.connect(lambda on: [cb.setChecked(on) for cb in self.chk.values()])
        grid.addWidget(self.all, 0, 0, 1, 2)
        self.chk = {}
        for i, (code, label, _) in enumerate(TRANSFORMS):
            cb = self.chk[code] = QCheckBox(label)
            cb.toggled.connect(lambda _=None: self.all.setChecked(all(c.isChecked() for c in self.chk.values())))
            grid.addWidget(cb, 1 + i // 2, i % 2)
        right.addWidget(box)
        box = QGroupBox("Kernels de desenfoque")
        form = QGridLayout(box)
        self.kg, self.kc = QSpinBox(), QSpinBox()
        for r, (text, sp) in enumerate((("Kernel gris:", self.kg), ("Kernel color:", self.kc))):
            sp.setRange(1, 99)
            sp.setSingleStep(2)
            sp.setValue(27)
            form.addWidget(QLabel(text), r, 0)
            form.addWidget(sp, r, 1)
        right.addWidget(box)
        right.addStretch()
        self.bar = QProgressBar()
        self.bar.setFormat("%v / %m imágenes")
        right.addWidget(self.bar)
        row = QHBoxLayout()
        self.run = QPushButton("Procesar")
        self.run.clicked.connect(self.start)
        self.stop = QPushButton("Cancelar")
        self.stop.setEnabled(False)
        self.stop.clicked.connect(lambda: self.proc and self.proc.kill())
        row.addWidget(self.run, 2)
        row.addWidget(self.stop, 1)
        right.addLayout(row)
        self.statusBar().showMessage(f"Programa esperado en: {EXE}")

    # ------------------------------------------------------------- archivos
    def say(self, text):
        self.log.appendPlainText(text)

    def pick(self):
        paths, _ = QFileDialog.getOpenFileNames(self, "Selecciona imágenes BMP", "", "BMP (*.bmp)")
        self.add_files(paths)

    def add_files(self, paths):
        for p in (str(Path(x).resolve()) for x in paths):
            if len(self.files) >= MAX_IMAGES:
                self.say(f"Límite de {MAX_IMAGES} imágenes.")
                break
            size = bmp_size(p)
            if p in self.files:
                continue
            if not size:
                self.say(f"Rechazado {Path(p).name}: debe ser BMP de 24 bits sin compresión.")
                continue
            if size[0] % 4:
                self.say(f"Aviso: {Path(p).name} mide {size[0]} px de ancho (no múltiplo de 4); las "
                         "inversiones de elemento.h ignoran el relleno de filas y el programa podría fallar.")
            icon = QIcon(QPixmap(p).scaled(100, 100, Qt.AspectRatioMode.KeepAspectRatio))
            self.list.addItem(QListWidgetItem(icon, f"{Path(p).name}\n{size[0]}×{size[1]}"))
            self.files.append(p)
        self.count.setText(f"{len(self.files)} / {MAX_IMAGES}")

    def clear(self):
        self.list.clear()
        self.files.clear()
        self.count.setText(f"0 / {MAX_IMAGES}")

    def stage(self):
        """Copia las imagenes como ImagenK.bmp y aparta las que ya hubiera."""
        d = WORK / "imagenes"
        bak = d / ".ui_respaldo"
        bak.mkdir(parents=True, exist_ok=True)
        for k in range(1, MAX_IMAGES + 1):
            p = d / f"Imagen{k}.bmp"
            if p.exists():
                p.unlink() if (bak / p.name).exists() else shutil.move(str(p), str(bak / p.name))
        for k, src in enumerate(self.files, 1):
            src_path = Path(src)
            if not src_path.exists() and (bak / src_path.name).exists():
                src_path = bak / src_path.name
            shutil.copyfile(src_path, d / f"Imagen{k}.bmp")

    def unstage(self):
        d = WORK / "imagenes"
        for k in range(1, MAX_IMAGES + 1):
            (d / f"Imagen{k}.bmp").unlink(missing_ok=True)
        bak = d / ".ui_respaldo"
        if bak.is_dir():
            for f in bak.iterdir():
                shutil.move(str(f), str(d / f.name))
            bak.rmdir()

    # ------------------------------------------------------------ ejecucion
    def ensure_exe(self):
        if EXE.exists():
            return True
        cmd = ["gcc", "-O2", "-fopenmp", "-o", str(EXE), str(SOURCE)]
        self.say("Compilando: " + " ".join(cmd))
        try:
            r = subprocess.run(cmd, cwd=HERE, capture_output=True, text=True, timeout=180)
        except (OSError, subprocess.TimeoutExpired) as exc:
            self.say(f"No se pudo ejecutar gcc: {exc}")
            return False
        if r.returncode:
            self.say(r.stderr.strip())
        return r.returncode == 0

    def start(self):
        if self.proc:
            return
        self.selected = [c for c, cb in self.chk.items() if cb.isChecked()]
        if not self.files or not self.selected:
            QMessageBox.information(self, "Faltan datos", "Agrega imágenes y marca al menos una transformación.")
            return
        self.log.clear()
        if not self.ensure_exe():
            QMessageBox.warning(self, "Programa no disponible", "No se pudo compilar transformadas.c.")
            return
        if "argc" not in SOURCE.read_text(errors="ignore"):
            self.say("Nota: este transformadas.c no lee argumentos; kernel y selección de transformaciones "
                     "no se aplican (usa su KERNEL_SIZE y todas las secciones).")
        shutil.rmtree(WORK / "resultados", ignore_errors=True)  # el programa tambien lo borra
        self.stage()
        self.bar.setRange(0, len(self.files))
        self.bar.setValue(0)
        self.combo.clear()
        self.buf.clear()
        self.out = []
        self.proc = QProcess(self)
        self.proc.setWorkingDirectory(str(WORK))
        self.proc.setProcessChannelMode(QProcess.ProcessChannelMode.MergedChannels)
        self.proc.readyRead.connect(self.read)
        self.proc.finished.connect(self.done)
        self.proc.errorOccurred.connect(lambda e: self.done(-1, None) if e == QProcess.ProcessError.FailedToStart else None)
        self.proc.start(str(EXE), ["--kernel-gray", str(self.kg.value()), "--kernel-color", str(self.kc.value()),
                                   "--transforms", ",".join(self.selected)])
        self.say(f"Procesando {len(self.files)} imagen(es)…")
        self.run.setEnabled(False)
        self.stop.setEnabled(True)
        self.timer.start()
        self.tabs.setCurrentIndex(0)

    def read(self):
        self.buf.append(self.proc.readAll())
        while (i := self.buf.indexOf(b"\n")) >= 0:
            line = bytes(self.buf[:i]).decode("utf-8", "replace").rstrip()
            self.buf.remove(0, i + 1)
            self.out.append(line)
            self.say(line)

    def poll(self):
        """Lo ultimo que mueve transformadas.c por imagen es edge_y.bmp."""
        self.bar.setValue(sum((WORK / "resultados" / f"Imagen{k}" / "edge_y.bmp").exists()
                              for k in range(1, len(self.files) + 1)))

    def done(self, code, status):
        if self.proc is None:
            return
        self.read()
        if status == QProcess.ExitStatus.CrashExit:
            self.say("Proceso cancelado o terminado con error.")
        elif code:
            self.say(f"No se pudo ejecutar el programa (código {code}): {EXE}")
        self.timer.stop()
        self.poll()
        self.unstage()
        self.proc.deleteLater()
        self.proc = None
        self.run.setEnabled(True)
        self.stop.setEnabled(False)
        # resumen: lineas finales que imprime transformadas.c
        keys = ("Imagenes procesadas", "Hilos disponibles", "Tiempo transcurrido")
        self.summary.setText(" · ".join(l for l in self.out if l.startswith(keys)) or "Sin resumen.")
        self.combo.blockSignals(True)
        for k, src in enumerate(self.files, 1):
            if (WORK / "resultados" / f"Imagen{k}").is_dir():
                self.combo.addItem(Path(src).name, k)
        self.combo.blockSignals(False)
        if self.combo.count():
            self.show_result()
            self.tabs.setCurrentIndex(1)
        else:
            self.say("No se generaron resultados; revisa los mensajes del programa arriba.")

    # ------------------------------------------------------------ resultados
    def show_result(self, *_):
        k = self.combo.currentData()
        if k is None:
            return
        folder = WORK / "resultados" / f"Imagen{k}"
        while self.gallery.count():
            self.gallery.takeAt(0).widget().deleteLater()
        cards = [("Original", self.files[k - 1]), ("Base en gris", folder / "base_gray.bmp")]
        cards += [(label, folder / fname) for code, label, fname in TRANSFORMS if code in self.selected]
        for i, (name, path) in enumerate(c for c in cards if Path(c[1]).exists()):
            pm = QPixmap(str(path)).scaled(150, 150, Qt.AspectRatioMode.KeepAspectRatio,
                                           Qt.TransformationMode.SmoothTransformation)
            btn = QToolButton()
            btn.setIcon(QIcon(pm))
            btn.setIconSize(QSize(150, 150))
            btn.setText(name)
            btn.setToolButtonStyle(Qt.ToolButtonStyle.ToolButtonTextUnderIcon)
            btn.setToolTip("Clic para abrir la imagen")
            btn.clicked.connect(lambda _=False, p=path: QDesktopServices.openUrl(QUrl.fromLocalFile(str(p))))
            self.gallery.addWidget(btn, i // 3, i % 3)

    def closeEvent(self, event):
        if self.proc:
            self.proc.kill()
            self.proc.waitForFinished(1000)
            self.done(-1, QProcess.ExitStatus.CrashExit)
        event.accept()


if __name__ == "__main__":
    app = QApplication(sys.argv)
    app.setStyle("Fusion")
    win = MainWindow()
    win.show()
    sys.exit(app.exec())
