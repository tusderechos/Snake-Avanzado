#include "audio.h"

#include <QApplication>
#include <QAudioOutput>
#include <QEvent>
#include <QMediaPlayer>
#include <QPushButton>
#include <QRandomGenerator>
#include <QUrl>

namespace {
const QString PREFIJO = QStringLiteral("qrc:/assets/audio/");
}

AudioManager &AudioManager::instancia()
{
    static AudioManager administrador;
    return administrador;
}

AudioManager::AudioManager(QObject *parent)
    : QObject(parent)
    , m_reproductorMusica(new QMediaPlayer(this))
    , m_salidaMusica(new QAudioOutput(this))
    , m_siguienteEfecto(0)
    , m_categoriaMusica(CategoriaMusica::Ninguna)
    , m_inicializado(false)
{
    m_pistasMenu = {
        QStringLiteral("menu_background_music1.mp3"),
        QStringLiteral("menu_background_music2.mp3"),
        QStringLiteral("menu_background_music3.mp3"),
        QStringLiteral("menu_background_music4.mp3")
    };
    m_pistasJuego = {
        QStringLiteral("game_background_music1.mp3"),
        QStringLiteral("game_background_music2.mp3"),
        QStringLiteral("game_background_music3.mp3")
    };
    m_pistaTienda = QStringLiteral("tienda_background_music.mp3");

    m_reproductorMusica->setAudioOutput(m_salidaMusica);
    m_reproductorMusica->setLoops(QMediaPlayer::Infinite);

    // Un reproductor por canal permite que varios efectos se superpongan.
    constexpr int CANALES_EFECTOS = 6;
    for (int i = 0; i < CANALES_EFECTOS; ++i) {
        auto *reproductor = new QMediaPlayer(this);
        auto *salida = new QAudioOutput(this);
        reproductor->setAudioOutput(salida);
        m_reproductoresEfectos.append(reproductor);
        m_salidasEfectos.append(salida);
    }
}

void AudioManager::inicializar()
{
    if (m_inicializado) return;
    m_inicializado = true;
    qApp->installEventFilter(this);
}

QString AudioManager::rutaRecurso(const QString &nombre) const
{
    return PREFIJO + nombre;
}

QString AudioManager::pistaAleatoria(const QVector<QString> &pistas) const
{
    if (pistas.isEmpty()) return {};
    const int indice = QRandomGenerator::global()->bounded(pistas.size());
    return pistas.at(indice);
}

void AudioManager::reproducirMusica(const QVector<QString> &pistas,
                                     CategoriaMusica categoria)
{
    if (m_categoriaMusica == categoria
        && m_reproductorMusica->playbackState() == QMediaPlayer::PlayingState) {
        return;
    }
    const QString pista = pistaAleatoria(pistas);
    if (pista.isEmpty()) return;

    m_categoriaMusica = categoria;
    m_reproductorMusica->stop();
    m_reproductorMusica->setSource(QUrl(rutaRecurso(pista)));
    m_reproductorMusica->play();
}

void AudioManager::reproducirMenu()
{
    detenerEfectosDePartida();
    reproducirMusica(m_pistasMenu, CategoriaMusica::Menu);
}

void AudioManager::reproducirJuego()
{
    detenerEfectosDePartida();
    reproducirMusica(m_pistasJuego, CategoriaMusica::Juego);
}

void AudioManager::reproducirTienda()
{
    detenerEfectosDePartida();
    if (m_categoriaMusica == CategoriaMusica::Tienda
        && m_reproductorMusica->playbackState() == QMediaPlayer::PlayingState) {
        return;
    }
    m_categoriaMusica = CategoriaMusica::Tienda;
    m_reproductorMusica->stop();
    m_reproductorMusica->setSource(QUrl(rutaRecurso(m_pistaTienda)));
    m_reproductorMusica->play();
}

void AudioManager::detenerMusica()
{
    m_reproductorMusica->stop();
    m_categoriaMusica = CategoriaMusica::Ninguna;
}

void AudioManager::detenerEfectos()
{
    for (QMediaPlayer *reproductor : m_reproductoresEfectos) {
        reproductor->stop();
    }
}

void AudioManager::detenerEfectosDePartida()
{
    const int limite = qMax(0, m_reproductoresEfectos.size() - 1);
    for (int i = 0; i < limite; ++i) {
        m_reproductoresEfectos.at(i)->stop();
    }
}

void AudioManager::reproducirEfecto(Efecto efecto)
{
    QString nombre;
    switch (efecto) {
    case Efecto::Boton: nombre = QStringLiteral("button_click.mp3"); break;
    case Efecto::Comer: nombre = QStringLiteral("snake_eat.mp3"); break;
    case Efecto::Moneda: nombre = QStringLiteral("coin_pickup.mp3"); break;
    case Efecto::Ganar: nombre = QStringLiteral("level_complete.mp3"); break;
    case Efecto::Perder: nombre = QStringLiteral("game_over.mp3"); break;
    }

    if (m_reproductoresEfectos.isEmpty()) return;
    const bool esBoton = efecto == Efecto::Boton;
    const int canalesPartida = qMax(1, m_reproductoresEfectos.size() - 1);
    const int canal = esBoton ? m_reproductoresEfectos.size() - 1
                              : m_siguienteEfecto % canalesPartida;
    if (!esBoton) ++m_siguienteEfecto;
    QMediaPlayer *reproductor = m_reproductoresEfectos.at(canal);
    reproductor->stop();
    reproductor->setSource(QUrl(rutaRecurso(nombre)));
    reproductor->play();
}

void AudioManager::establecerVolumenMusica(int volumen)
{
    m_salidaMusica->setVolume(qBound(0.0, volumen / 100.0, 1.0));
}

void AudioManager::establecerVolumenSonido(int volumen)
{
    const qreal nivel = qBound(0.0, volumen / 100.0, 1.0);
    for (QAudioOutput *salida : m_salidasEfectos) salida->setVolume(nivel);
}

bool AudioManager::eventFilter(QObject *objeto, QEvent *evento)
{
    if (evento->type() == QEvent::MouseButtonPress
        && qobject_cast<QPushButton *>(objeto) != nullptr) {
        reproducirEfecto(Efecto::Boton);
    }
    return QObject::eventFilter(objeto, evento);
}
