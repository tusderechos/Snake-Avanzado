#ifndef MANUALUSUARIO_H
#define MANUALUSUARIO_H
#include <QWidget>
#include <QStringList>
class QLabel;
class QPushButton;
class QTimer;
class ManualUsuario : public QWidget {
public:
    explicit ManualUsuario(QWidget *parent = nullptr);
private:
    void mostrarPagina(int pagina);
    QStringList paginas;
    QLabel *paginaImagen;
    QLabel *textoIntro;
    QLabel *shrekIntro;
    QLabel *numeroPagina;
    QPushButton *anterior;
    QPushButton *siguiente;
    QPushButton *cerrarManual;
    QTimer *escrituraTimer;
    QString textoIntroduccion;
    int caracteresIntroduccion = 0;
    int paginaActual = 0;
};
#endif
