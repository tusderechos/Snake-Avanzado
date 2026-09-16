#ifndef AUDIO_H
#define AUDIO_H

#include <QObject>
#include <QString>
#include <QVector>

class QAudioOutput;
class QEvent;
class QMediaPlayer;

class AudioManager final : public QObject
{
public:
    enum class Efecto {
        Boton,
        Comer,
        Moneda,
        Ganar,
        Perder
    };

    static AudioManager &instancia();

    void inicializar();
    void reproducirMenu();
    void reproducirJuego();
    void reproducirTienda();
    void detenerMusica();
    void detenerEfectos();
    void detenerEfectosDePartida();
    void reproducirEfecto(Efecto efecto);
    void establecerVolumenMusica(int volumen);
    void establecerVolumenSonido(int volumen);

protected:
    bool eventFilter(QObject *objeto, QEvent *evento) override;

private:
    enum class CategoriaMusica { Ninguna, Menu, Juego, Tienda };

    explicit AudioManager(QObject *parent = nullptr);
    void reproducirMusica(const QVector<QString> &pistas,
                          CategoriaMusica categoria);
    QString pistaAleatoria(const QVector<QString> &pistas) const;
    QString rutaRecurso(const QString &nombre) const;

    QMediaPlayer *m_reproductorMusica;
    QAudioOutput *m_salidaMusica;
    QVector<QMediaPlayer *> m_reproductoresEfectos;
    QVector<QAudioOutput *> m_salidasEfectos;
    QVector<QString> m_pistasMenu;
    QVector<QString> m_pistasJuego;
    QString m_pistaTienda;
    int m_siguienteEfecto;
    CategoriaMusica m_categoriaMusica;
    bool m_inicializado;
};

#endif // AUDIO_H
