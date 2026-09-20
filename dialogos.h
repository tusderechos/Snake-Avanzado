#ifndef DIALOGOS_H
#define DIALOGOS_H

#include <QMessageBox>
#include <QLabel>
#include <QPainter>
#include <QSizePolicy>

namespace
{
QPixmap crearIconoDialogo(QMessageBox::Icon icono)
{
    QColor color("#68dd3e");
    QString simbolo = "i";
    if (icono == QMessageBox::Critical) {
        color = QColor("#e85d4a");
        simbolo = "×";
    } else if (icono == QMessageBox::Warning) {
        color = QColor("#e8b84a");
        simbolo = "!";
    } else if (icono == QMessageBox::Question) {
        color = QColor("#5db8e8");
        simbolo = "?";
    }

    QPixmap pixmap(72, 72);
    pixmap.fill(Qt::transparent);
    QPainter pintor(&pixmap);
    pintor.setRenderHint(QPainter::Antialiasing);
    pintor.setPen(QPen(QColor("#f8edb6"), 3));
    pintor.setBrush(color);
    pintor.drawEllipse(5, 5, 62, 62);
    pintor.setPen(Qt::white);
    pintor.setFont(QFont("Fredoka", 34, QFont::Bold));
    pintor.drawText(pixmap.rect(), Qt::AlignCenter, simbolo);
    return pixmap;
}
}

namespace Dialogos
{
inline void aplicarEstilo(QMessageBox &dialogo)
{
    dialogo.setIconPixmap(crearIconoDialogo(dialogo.icon()));
    dialogo.setMinimumSize(720, 250);
    dialogo.setMaximumWidth(820);
    for (QLabel *etiqueta : dialogo.findChildren<QLabel *>()) {
        if (etiqueta->objectName() == "qt_msgbox_label"
            || etiqueta->objectName() == "qt_msgbox_informativelabel") {
            etiqueta->setWordWrap(true);
            etiqueta->setMinimumWidth(520);
            etiqueta->setMaximumWidth(650);
            etiqueta->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        }
    }
    dialogo.setStyleSheet(
        "QMessageBox { background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        " stop:0 #1b2b16, stop:0.48 #10180f, stop:1 #081008);"
        " color: #f4f7ed; border: 3px solid #d2a93b; border-radius: 12px; }"
        "QLabel { color: #f4f7ed; font-family: 'Fredoka';"
        " font-size: 16px; padding: 4px; }"
        "QLabel#qt_msgbox_label { color: #fff7d1; font-size: 20px;"
        " font-weight: bold; }"
        "QLabel#qt_msgbox_informativelabel { color: #f4f7ed;"
        " font-size: 15px; }"
        "QPushButton { background-color: #2f8618; color: white;"
        " border: 2px solid #68dd3e; border-radius: 8px;"
        " font-family: 'Fredoka'; font-size: 16px; font-weight: bold;"
        " min-width: 130px; min-height: 38px; padding: 5px 14px; }"
        "QPushButton:hover { background-color: #4cae22;"
        " border-color: #d2a93b; }"
        "QPushButton:pressed { background-color: #236e0a;"
        " border-color: white; }"
        );
    dialogo.adjustSize();
}

inline void mostrar(QWidget *padre,
                    QMessageBox::Icon icono,
                    const QString &titulo,
                    const QString &texto)
{
    QMessageBox dialogo(padre);
    dialogo.setIcon(icono);
    dialogo.setWindowTitle(titulo);
    dialogo.setText(texto);
    aplicarEstilo(dialogo);
    dialogo.exec();
}
}

#endif // DIALOGOS_H
