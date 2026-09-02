#include "juegoview.h"
#include "menujuego.h"
#include "tutorial.h"

#include <QApplication>
#include <QMessageBox>
#include <QInputDialog>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QSpinBox>

int main(int argc, char *argv[]) {
    QApplication aplicacion(argc, argv);

    MenuJuego *menu = nullptr;
    menu = new MenuJuego([&menu](ModoJuego modo) {
        if (modo == ModoJuego::Tutorial) {
            auto *tutorial = new TutorialView();
            tutorial->setAttribute(Qt::WA_DeleteOnClose);
            menu->hide();
            QObject::connect(tutorial, &QObject::destroyed, menu, &QWidget::show);
            tutorial->show();
            return;
        }

        int nivelInicial = 1;
        ConfiguracionJuego configuracion;
        if (modo == ModoJuego::Libre) {
            bool aceptado = false;
            nivelInicial = QInputDialog::getInt(menu, "Modo Libre",
                                                "Elegí el nivel inicial:",
                                                1, 1, 3, 1, &aceptado);
            if (!aceptado) {
                return;
            }
            configuracion.progresionAutomatica = false;
        } else if (modo == ModoJuego::Aleatorio) {
            QDialog dialogo(menu);
            dialogo.setWindowTitle("Configuración aleatoria");
            dialogo.setStyleSheet("QDialog { background: #0d121a; color: #ebf0f5; }");
            auto *formulario = new QFormLayout(&dialogo);
            auto *nivel = new QSpinBox(&dialogo);
            nivel->setRange(1, 3);
            nivel->setValue(3);
            auto *frutas = new QCheckBox("Frutas especiales", &dialogo);
            auto *items = new QCheckBox("Cajas e ítems", &dialogo);
            auto *obstaculos = new QCheckBox("Obstáculos", &dialogo);
            auto *moviles = new QCheckBox("Obstáculos móviles", &dialogo);
            auto *azar = new QCheckBox("Aparición aleatoria", &dialogo);
            frutas->setChecked(true);
            items->setChecked(true);
            obstaculos->setChecked(true);
            moviles->setChecked(true);
            azar->setChecked(true);
            formulario->addRow("Nivel inicial:", nivel);
            formulario->addRow(frutas);
            formulario->addRow(items);
            formulario->addRow(obstaculos);
            formulario->addRow(moviles);
            formulario->addRow(azar);
            auto *botones = new QDialogButtonBox(QDialogButtonBox::Ok
                                                  | QDialogButtonBox::Cancel, &dialogo);
            formulario->addRow(botones);
            QObject::connect(botones, &QDialogButtonBox::accepted,
                             &dialogo, &QDialog::accept);
            QObject::connect(botones, &QDialogButtonBox::rejected,
                             &dialogo, &QDialog::reject);
            if (dialogo.exec() != QDialog::Accepted) {
                return;
            }
            nivelInicial = nivel->value();
            configuracion.frutasEspeciales = frutas->isChecked();
            configuracion.items = items->isChecked();
            configuracion.obstaculos = obstaculos->isChecked();
            configuracion.obstaculosMoviles = moviles->isChecked();
            configuracion.aparicionAleatoria = azar->isChecked();
        }

        auto *juego = new JuegoView(nivelInicial, configuracion);
        juego->setAttribute(Qt::WA_DeleteOnClose);
        menu->hide();
        QObject::connect(juego, &QObject::destroyed, menu, &QWidget::show);
        juego->show();
    });
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->show();

    return aplicacion.exec();
}
