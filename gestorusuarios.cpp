#include "gestorusuarios.h"
#include <QCoreApplication>
#include <QCryptographicHash>

namespace {
QString identificadorAuth(const QString &usuario) {
    return QString::fromLatin1(QCryptographicHash::hash(usuario.trimmed().toUtf8(), QCryptographicHash::Sha256).toHex()) + "@snake.invalid";
}
}

#include <QDateTime>
#include <QJsonArray>
#include <QTimer>
#include <QUuid>

GestorUsuarios::GestorUsuarios(QObject *p) : QObject(p) {}
GestorUsuarios &GestorUsuarios::instancia() {
    static auto *g = new GestorUsuarios(QCoreApplication::instance());
    return *g;
}
QString GestorUsuarios::nombreActual() { return instancia().m_perfil.value("nombre_usuario").toString(); }
bool GestorUsuarios::sesionActiva() { return !instancia().m_id.isEmpty() && !instancia().m_perfil.isEmpty(); }
bool GestorUsuarios::pendientes() { auto &g=instancia(); return !g.m_cola.empty() || g.m_autenticando; }
QJsonObject GestorUsuarios::perfil(const QString &u) {
    return sesionActiva() && u == nombreActual() ? instancia().m_perfil : QJsonObject{};
}
int GestorUsuarios::obtenerPuntosUsuario(const QString &u) { return perfil(u).value("puntos").toInt(); }
int GestorUsuarios::obtenerMonedasUsuario(const QString &u) { return perfil(u).value("monedas").toInt(); }
bool GestorUsuarios::tieneSkin(const QString &u,const QString &s) { return perfil(u).value("skins").toArray().contains(s); }
QString GestorUsuarios::obtenerSkinEquipada(const QString &u) { return perfil(u).value("skin_equipada").toString("clasica"); }
QString GestorUsuarios::obtenerAvatar(const QString &u) { return perfil(u).value("avatar").toString("clasica"); }
bool GestorUsuarios::tutorialCompletado(const QString &u) { return perfil(u).value("tutorial_completado").toBool(); }
int GestorUsuarios::obtenerNivelHistoria(const QString &u) { return perfil(u).value("nivel_historia").toInt(); }
bool GestorUsuarios::adoptarSesion(const QJsonObject &o) {
    const auto user=o.value("user").toObject();
    if(o.value("access_token").toString().isEmpty() || user.value("id").toString().isEmpty()) return false;
    if(!m_id.isEmpty() && user.value("id").toString()!=m_id) return false;
    m_token=o.value("access_token").toString(); m_refresh=o.value("refresh_token").toString();
    m_id=user.value("id").toString(); m_correo=user.value("email").toString();
    m_expira=QDateTime::currentSecsSinceEpoch()+o.value("expires_in").toInt(3600);
    return true;
}
void GestorUsuarios::registrarUsuario(const QString &u,const QString &pass,QObject *ctx,Respuesta cb) {
    SupabaseClient::solicitar("POST","/auth/v1/signup",{{"email",identificadorAuth(u)},{"password",pass},
        {"data",QJsonObject{{"nombre_usuario",u.trimmed()}}}}, {},ctx,[cb](auto r){
        if(!r.exito) {cb(false,r.mensaje); return;}
        const auto o=r.datos.toObject();
        if(o.value("user").toObject().value("id").toString().isEmpty() && o.value("id").toString().isEmpty()) {
            cb(false,"El servidor no devolvio la cuenta registrada."); return;
        }
        cb(!o.value("access_token").toString().isEmpty(),o.value("access_token").toString().isEmpty()
            ? QStringLiteral("El servidor aun exige confirmacion de correo. El administrador debe desactivar Confirm email en Supabase.")
            : QStringLiteral("Cuenta creada. Ya puede iniciar sesion con su usuario y contrasena."));
    });
}
void GestorUsuarios::iniciarSesion(const QString &usuario,const QString &pass,QObject *ctx,Respuesta cb) {
    auto &g=instancia();
    if(pendientes()) {cb(false,"Espere a que termine la operacion actual."); return;}
    cerrarSesion(); g.m_autenticando=true;
    QPointer<QObject> guard(ctx); auto gen=g.m_generacion;
    SupabaseClient::solicitar("POST","/auth/v1/token?grant_type=password",{{"email",identificadorAuth(usuario)},{"password",pass}}, {},&g,
        [&g,guard,cb,gen](auto r){
        if(gen!=g.m_generacion) return;
        if(!r.exito || !g.adoptarSesion(r.datos.toObject())) {
            g.m_autenticando=false; if(guard) cb(false,r.exito?"Respuesta de inicio de sesion incompleta.":r.mensaje); return;
        }
        SupabaseClient::solicitar("POST","/rest/v1/rpc/snake_perfil",{},g.m_token,&g,[&g,guard,cb,gen](auto p){
            if(gen!=g.m_generacion) return;
            g.m_autenticando=false;
            const auto o=p.datos.toObject();
            if(!guard || !p.exito || o.value("id").toString()!=g.m_id || o.value("nombre_usuario").toString().isEmpty()) {
                cerrarSesion(); if(guard) cb(false,p.exito?"No se pudo cargar el perfil de esta cuenta.":p.mensaje); return;
            }
            g.m_perfil=o; emit g.perfilActualizado(); cb(true,{});
        });
    });
}
void GestorUsuarios::conToken(std::function<void(bool,QString)> cb) {
    if(m_token.isEmpty()) {cb(false,"Inicie sesion de nuevo."); return;}
    if(m_expira>QDateTime::currentSecsSinceEpoch()+60) {cb(true,{}); return;}
    const auto gen=m_generacion;
    SupabaseClient::solicitar("POST","/auth/v1/token?grant_type=refresh_token",{{"refresh_token",m_refresh}}, {},this,
        [this,gen,cb](auto r){
        if(gen!=m_generacion) return;
        cb(r.exito && adoptarSesion(r.datos.toObject()),r.exito?"No se pudo renovar la sesion.":r.mensaje);
    });
}
bool GestorUsuarios::encolar(const QString &u,const QString &ruta,const QJsonObject &body,const QByteArray &metodo) {
    auto &g=instancia();
    if(!sesionActiva() || u!=nombreActual()) {emit g.errorGuardado("No hay una sesion valida para guardar estos datos."); return false;}
    g.m_cola.push_back({metodo,ruta,body}); emit g.pendientesCambiaron();
    QTimer::singleShot(0,&g,[&g]{g.procesar();}); return true;
}
void GestorUsuarios::procesar() {
    if(m_procesando || m_fallido || m_cola.empty() || m_autenticando) return;
    m_procesando=true; const auto gen=m_generacion;
    conToken([this,gen](bool ok,QString error){
        if(gen!=m_generacion) return;
        if(!ok) {m_procesando=false; m_fallido=true; emit errorGuardado(error); return;}
        const auto job=m_cola.front();
        SupabaseClient::solicitar(job.metodo,job.ruta,job.cuerpo,m_token,this,[this,gen](auto r){
            if(gen!=m_generacion) return;
            m_procesando=false;
            auto o=r.datos.toObject();
            if(r.datos.isArray() && !r.datos.toArray().isEmpty()) o=r.datos.toArray().first().toObject();
            if(!r.exito || o.value("id").toString()!=m_id) {
                const bool temporal=r.codigo==0 || r.codigo>=500 || r.codigo==429 || r.codigo==401;
                m_fallido=temporal;
                if(r.codigo==401) m_expira=0;
                if(!temporal) m_cola.pop_front();
                emit errorGuardado(r.exito?"El servidor no devolvio el perfil guardado.":r.mensaje);
                emit pendientesCambiaron();
                if(!temporal) QTimer::singleShot(0,this,[this]{procesar();});
                return;
            }
            m_perfil=o; m_cola.pop_front(); emit perfilActualizado(); emit pendientesCambiaron(); procesar();
        });
    });
}
void GestorUsuarios::reintentar() {auto &g=instancia(); g.m_fallido=false; g.procesar();}
bool GestorUsuarios::cerrarSesion() {
    auto &g=instancia(); if(pendientes()) return false;
    const auto token=g.m_token;
    ++g.m_generacion; g.m_token.clear(); g.m_refresh.clear(); g.m_id.clear(); g.m_correo.clear(); g.m_perfil={};
    g.m_expira=0; g.m_fallido=false;
    if(!token.isEmpty()) SupabaseClient::solicitar("POST","/auth/v1/logout?scope=local",{},token,&g,[](auto){});
    emit g.perfilActualizado(); return true;
}
bool GestorUsuarios::registrarPuntajePartida(const QString &u,int p,int b) {
    if(p<0 || b<0 || (p==0 && b==0)) return false;
    return encolar(u,"/rest/v1/rpc/snake_partida",{{"p_operacion",QUuid::createUuid().toString(QUuid::WithoutBraces)}, {"p_puntos",p},{"p_bonus",b}});
}
bool GestorUsuarios::comprarSkin(const QString &u,const QString &s,int) {
    if (s == "thanos" && obtenerNivelHistoria(u) < 3) {
        emit instancia().errorGuardado("Thanos se desbloquea al completar los 3 niveles de HISTORIA.");
        return false;
    }
    return encolar(u,"/rest/v1/rpc/snake_comprar_skin",{{"p_skin",s}});
}
bool GestorUsuarios::equiparSkin(const QString &u,const QString &s) {return encolar(u,"/rest/v1/rpc/snake_equipar_skin",{{"p_skin",s}});}
bool GestorUsuarios::marcarTutorialCompletado(const QString &u) {return encolar(u,"/rest/v1/rpc/snake_tutorial",{});}
bool GestorUsuarios::marcarNivelHistoriaCompletado(const QString &u,int n) {return encolar(u,"/rest/v1/rpc/snake_nivel",{{"p_nivel",n}});}
bool GestorUsuarios::guardarPreferencias(const QString &u,const QJsonObject &c) {
    for(auto i=c.begin();i!=c.end();++i) {
        if(i.key()=="control") {if(i.value()!="WASD" && i.value()!="FLECHAS") return false;}
        else if(i.key()=="volumen_musica" || i.key()=="volumen_sonido") {if(!i.value().isDouble() || i.value().toDouble()<0 || i.value().toDouble()>1) return false;}
        else if(i.key()=="avatar") {const QString a=i.value().toString(); if(a!="clasica" && a!="gato" && a!="dragon" && a!="burro" && a!="spiderman" && a!="miles" && a!="personaje" && a!="thanos") return false;}
        else return false;
    }
    return encolar(u,"/rest/v1/perfiles?id=eq."+instancia().m_id,c,"PATCH");
}
bool GestorUsuarios::guardarAvatar(const QString &u,const QString &avatar) {
    return guardarPreferencias(u, {{"avatar", avatar}});
}
void GestorUsuarios::cambiarContrasena(const QString &u,const QString &oldPass,const QString &newPass,QObject *ctx,Respuesta cb) {
    auto &g=instancia();
    if(!sesionActiva() || u!=nombreActual() || pendientes()) {cb(false,"Espere a que finalice el guardado y compruebe su sesion."); return;}
    g.m_autenticando=true; const auto gen=g.m_generacion; QPointer<QObject> guard(ctx);
    SupabaseClient::solicitar("POST","/auth/v1/token?grant_type=password",{{"email",g.m_correo},{"password",oldPass}}, {},&g,
        [&g,gen,guard,newPass,cb](auto r){
        if(gen!=g.m_generacion) return;
        if(!r.exito || !g.adoptarSesion(r.datos.toObject())) {g.m_autenticando=false; if(guard) cb(false,r.mensaje); g.procesar(); return;}
        SupabaseClient::solicitar("PUT","/auth/v1/user",{{"password",newPass}},g.m_token,&g,[&g,gen,guard,cb](auto rr){
            if(gen!=g.m_generacion) return;
            g.m_autenticando=false; if(guard) cb(rr.exito,rr.mensaje); g.procesar();
        });
    });
}
void GestorUsuarios::obtenerRanking(int n,QObject *ctx,std::function<void(bool,const QString &,QVector<DatoRanking>)> cb) {
    SupabaseClient::solicitar("GET","/rest/v1/ranking_publico?select=nombre_usuario,puntos&order=puntos.desc,nombre_usuario.asc&limit="+QString::number(qBound(1,n,100)),{}, {},ctx,[cb](auto r){
        QVector<DatoRanking> datos;
        if(r.exito && r.datos.isArray()) for(const auto &v:r.datos.toArray()) {auto o=v.toObject(); datos.append({o.value("nombre_usuario").toString(),o.value("puntos").toInt()});}
        cb(r.exito && r.datos.isArray(),r.mensaje,datos);
    });
}
