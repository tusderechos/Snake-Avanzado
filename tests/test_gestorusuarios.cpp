#include "gestorusuarios.h"
#include <QtTest>
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonArray>
#include <QPointer>
#include <memory>
#include <QCryptographicHash>

class GestorUsuariosTest : public QObject {
    Q_OBJECT
    struct Request { QByteArray headers; QJsonObject body; QPointer<QTcpSocket> socket; };
    QTcpServer server;
    QList<Request> requests;
    QJsonObject profile(QString id = "account-a", QString name = "Ana", int points = 12) {
        return {{"id",id},{"nombre_usuario",name},{"puntos",points},{"monedas",7},
                {"skins",QJsonArray{"clasica"}},{"skin_equipada","clasica"},
                {"tutorial_completado",true},{"nivel_historia",2},{"volumen_musica",0.5}};
    }
    QJsonObject session(QString id = "account-a", QString token = "jwt-a", int seconds = 3600) {
        return {{"access_token",token},{"refresh_token","refresh-a"},{"expires_in",seconds},
                {"user",QJsonObject{{"id",id},{"email",id+"@example.test"}}}};
    }
    void send(int index, QJsonValue value, int status = 200) {
        auto socket=requests.at(index).socket;
        QVERIFY(socket);
        QByteArray body=value.isArray() ? QJsonDocument(value.toArray()).toJson(QJsonDocument::Compact)
                                       : QJsonDocument(value.toObject()).toJson(QJsonDocument::Compact);
        socket->write("HTTP/1.1 "+QByteArray::number(status)+" Test\r\nContent-Type: application/json\r\nContent-Length: "+QByteArray::number(body.size())+"\r\nConnection: close\r\n\r\n"+body);
        socket->disconnectFromHost();
    }
    void login(QString id = "account-a", QString name = "Ana", int expires = 3600) {
        int calls=0; bool ok=false; const int start=requests.size();
        GestorUsuarios::iniciarSesion(id+"@example.test","password",this,[&](bool success,const QString &){ok=success;++calls;});
        QTRY_COMPARE(requests.size(),start+1);
        QVERIFY(requests[start].headers.startsWith("POST /auth/v1/token?grant_type=password "));
        send(start,session(id,"jwt-"+id,expires));
        QTRY_COMPARE(requests.size(),start+2);
        QVERIFY(requests[start+1].headers.startsWith("POST /rest/v1/rpc/snake_perfil "));
        QVERIFY(!GestorUsuarios::sesionActiva()); QCOMPARE(calls,0);
        send(start+1,profile(id,name));
        QTRY_COMPARE(calls,1); QVERIFY(ok); QVERIFY(GestorUsuarios::sesionActiva());
    }
private slots:
    void init() {
        QVERIFY(!GestorUsuarios::pendientes()); QVERIFY(GestorUsuarios::cerrarSesion());
        requests.clear(); QVERIFY(server.listen(QHostAddress::LocalHost));
        qputenv("SNAKE_SUPABASE_URL", QByteArray("http://127.0.0.1:")+QByteArray::number(server.serverPort()));
        qputenv("SNAKE_SUPABASE_KEY","test-public");
        connect(&server,&QTcpServer::newConnection,this,[this] {
            auto *socket=server.nextPendingConnection();
            auto buffer=std::make_shared<QByteArray>();
            connect(socket,&QTcpSocket::readyRead,this,[this,socket,buffer] {
                *buffer+=socket->readAll(); const int boundary=buffer->indexOf("\r\n\r\n");
                if(boundary<0) return;
                const QByteArray headers=buffer->left(boundary); int length=0;
                for(const auto &line:headers.split('\n')) if(line.toLower().startsWith("content-length:")) length=line.mid(15).trimmed().toInt();
                if(buffer->size()<boundary+4+length) return;
                socket->disconnect(this);
                if(headers.startsWith("POST /auth/v1/logout")) {
                    socket->write("HTTP/1.1 204 No Content\r\nConnection: close\r\n\r\n"); socket->disconnectFromHost(); return;
                }
                requests.append({headers,QJsonDocument::fromJson(buffer->mid(boundary+4,length)).object(),socket});
            });
            connect(socket,&QTcpSocket::disconnected,socket,&QObject::deleteLater);
        });
    }
    void cleanup() {
        QVERIFY(!GestorUsuarios::pendientes()); QVERIFY(GestorUsuarios::cerrarSesion());
        QTest::qWait(30); server.close(); server.disconnect(this);
        qunsetenv("SNAKE_SUPABASE_URL"); qunsetenv("SNAKE_SUPABASE_KEY");
    }
    void loginWaitsForProfileAndLogoutClearsAccount() {
        login(); QCOMPARE(GestorUsuarios::obtenerPuntosUsuario("Ana"),12);
        QVERIFY(GestorUsuarios::cerrarSesion()); QVERIFY(!GestorUsuarios::sesionActiva());
        QVERIFY(GestorUsuarios::perfil("Ana").isEmpty());
        login("account-b","Beto");
        QVERIFY(GestorUsuarios::perfil("Ana").isEmpty()); QCOMPARE(GestorUsuarios::nombreActual(),QString("Beto"));
    }
    void usernameRegistrationAndLoginUseSameIdentifier() {
        int calls=0; bool ok=false;
        GestorUsuarios::registrarUsuario("Jugador_1","Segura123!",this,[&](bool success,const QString &){ok=success;++calls;});
        QTRY_COMPARE(requests.size(),1);
        const QString expected=QString::fromLatin1(QCryptographicHash::hash("Jugador_1",QCryptographicHash::Sha256).toHex())+"@snake.invalid";
        QCOMPARE(requests[0].body.value("email").toString(),expected);
        QCOMPARE(requests[0].body.value("data").toObject().value("nombre_usuario").toString(),QString("Jugador_1"));
        send(0,session());
        QTRY_COMPARE(calls,1); QVERIFY(ok);
        GestorUsuarios::iniciarSesion("Jugador_1","Segura123!",this,[&](bool success,const QString &){ok=success;++calls;});
        QTRY_COMPARE(requests.size(),2);
        QCOMPARE(requests[1].body.value("email").toString(),expected);
        send(1,session()); QTRY_COMPARE(requests.size(),3); send(2,profile("account-a","Jugador_1"));
        QTRY_COMPARE(calls,2); QVERIFY(ok);
    }
    void mutationWaitsForServerAndBlocksLogout() {
        login(); const int start=requests.size();
        QVERIFY(GestorUsuarios::registrarPuntajePartida("Ana",8,2)); QVERIFY(GestorUsuarios::pendientes());
        QVERIFY(!GestorUsuarios::cerrarSesion()); QCOMPARE(GestorUsuarios::obtenerPuntosUsuario("Ana"),12);
        QTRY_COMPARE(requests.size(),start+1);
        QCOMPARE(requests[start].body.value("p_puntos").toInt(),8); QVERIFY(!requests[start].body.value("p_operacion").toString().isEmpty());
        QCOMPARE(GestorUsuarios::obtenerPuntosUsuario("Ana"),12);
        send(start,profile("account-a","Ana",20));
        QTRY_VERIFY(!GestorUsuarios::pendientes()); QCOMPARE(GestorUsuarios::obtenerPuntosUsuario("Ana"),20);
    }
    void preferencesSendOnlyChangedFields() {
        login(); const int start=requests.size();
        QVERIFY(GestorUsuarios::guardarPreferencias("Ana",{{"volumen_musica",0.8}}));
        QTRY_COMPARE(requests.size(),start+1);
        QVERIFY(requests[start].headers.startsWith("PATCH /rest/v1/perfiles?id=eq.account-a "));
        QCOMPARE(requests[start].body,QJsonObject({{"volumen_musica",0.8}}));
        auto updated=profile(); updated["volumen_musica"]=0.8; send(start,QJsonArray{updated});
        QTRY_VERIFY(!GestorUsuarios::pendientes()); QCOMPARE(GestorUsuarios::perfil("Ana").value("volumen_musica").toDouble(),0.8);
    }
    void transientRetryReusesOperationId() {
        login(); const int start=requests.size(); QSignalSpy errors(&GestorUsuarios::instancia(),&GestorUsuarios::errorGuardado);
        QVERIFY(GestorUsuarios::registrarPuntajePartida("Ana",8)); QTRY_COMPARE(requests.size(),start+1);
        const auto operation=requests[start].body.value("p_operacion"); send(start,QJsonObject{{"message","temporary"}},503);
        QTRY_COMPARE(errors.size(),1); QVERIFY(GestorUsuarios::pendientes()); QCOMPARE(GestorUsuarios::obtenerPuntosUsuario("Ana"),12);
        GestorUsuarios::reintentar(); QTRY_COMPARE(requests.size(),start+2);
        QCOMPARE(requests[start+1].body.value("p_operacion"),operation);
        send(start+1,profile("account-a","Ana",20)); QTRY_VERIFY(!GestorUsuarios::pendientes());
        QCOMPARE(GestorUsuarios::obtenerPuntosUsuario("Ana"),20);
    }
    void wrongAccountMutationNeverSendsRequest() {
        login(); const int start=requests.size();
        QVERIFY(!GestorUsuarios::registrarPuntajePartida("Beto",10)); QTest::qWait(40);
        QCOMPARE(requests.size(),start); QVERIFY(!GestorUsuarios::pendientes());
    }
    void authFailureLeavesNoSession() {
        int calls=0; bool ok=true;
        GestorUsuarios::iniciarSesion("a@example.test","wrong",this,[&](bool success,const QString &){ok=success;++calls;});
        QTRY_COMPARE(requests.size(),1); send(0,QJsonObject{{"error_code","invalid_credentials"}},400);
        QTRY_COMPARE(calls,1); QVERIFY(!ok); QVERIFY(!GestorUsuarios::sesionActiva()); QVERIFY(GestorUsuarios::nombreActual().isEmpty());
    }
    void destroyedLoginContextCannotAdoptProfile() {
        int calls=0; auto *context=new QObject;
        GestorUsuarios::iniciarSesion("a@example.test","password",context,[&](bool,const QString &){++calls;});
        QTRY_COMPARE(requests.size(),1); send(0,session()); QTRY_COMPARE(requests.size(),2);
        delete context; send(1,profile()); QTRY_VERIFY(!GestorUsuarios::pendientes());
        QVERIFY(!GestorUsuarios::sesionActiva()); QCOMPARE(calls,0);
    }
    void expiredTokenRefreshesBeforeMutation() {
        login("account-a","Ana",1); const int start=requests.size();
        QVERIFY(GestorUsuarios::marcarTutorialCompletado("Ana")); QTRY_COMPARE(requests.size(),start+1);
        QVERIFY(requests[start].headers.startsWith("POST /auth/v1/token?grant_type=refresh_token "));
        QCOMPARE(requests[start].body.value("refresh_token").toString(),QString("refresh-a"));
        QVERIFY(!requests[start].headers.toLower().contains("authorization:")); send(start,session("account-a","refreshed-user-jwt"));
        QTRY_COMPARE(requests.size(),start+2);
        QVERIFY(requests[start+1].headers.contains("Authorization: Bearer refreshed-user-jwt"));
        send(start+1,profile()); QTRY_VERIFY(!GestorUsuarios::pendientes());
    }
};
QTEST_GUILESS_MAIN(GestorUsuariosTest)
#include "test_gestorusuarios.moc"

