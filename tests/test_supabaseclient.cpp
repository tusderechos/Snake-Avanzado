#include "supabaseclient.h"
#include <QtTest>
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonArray>

class TransportTest : public QObject {
    Q_OBJECT
    QTcpServer server;
    QByteArray request;
    QByteArray response;
    bool hold = false;
private slots:
    void init() {
        QVERIFY(server.listen(QHostAddress::LocalHost));
        qputenv("SNAKE_SUPABASE_URL", QByteArray("http://127.0.0.1:") + QByteArray::number(server.serverPort()));
        qputenv("SNAKE_SUPABASE_KEY", "public-test-key");
        request.clear(); response = "HTTP/1.1 200 OK\r\nContent-Length: 2\r\nConnection: close\r\n\r\n{}"; hold = false;
        connect(&server, &QTcpServer::newConnection, this, [this] {
            auto *socket = server.nextPendingConnection();
            connect(socket, &QTcpSocket::readyRead, this, [this, socket] {
                request += socket->readAll();
                if (request.contains("\r\n\r\n") && !hold) { socket->write(response); socket->disconnectFromHost(); }
            });
            connect(socket, &QTcpSocket::disconnected, socket, &QObject::deleteLater);
        });
    }
    void cleanup() { server.close(); server.disconnect(this); qunsetenv("SNAKE_SUPABASE_URL"); qunsetenv("SNAKE_SUPABASE_KEY"); qunsetenv("SNAKE_SUPABASE_TIMEOUT_MS"); }
    void publicRequestNeverSendsAuthorization() {
        int calls = 0; SupabaseClient::Resultado result;
        SupabaseClient::solicitar("POST", "/auth/v1/signup", {}, {}, this, [&](auto r) { result=r; ++calls; });
        QCOMPARE(calls, 0); QTRY_COMPARE(calls, 1); QVERIFY(result.exito);
        QVERIFY(request.toLower().contains("apikey: public-test-key")); QVERIFY(!request.toLower().contains("authorization:"));
    }
    void authenticatedRequestPreservesArray() {
        response = "HTTP/1.1 200 OK\r\nContent-Length: 10\r\nConnection: close\r\n\r\n[{\"id\":1}]";
        int calls = 0; SupabaseClient::Resultado result;
        SupabaseClient::solicitar("GET", "/rest/v1/perfiles", {}, "user-jwt", this, [&](auto r) {result=r; ++calls;});
        QTRY_COMPARE(calls, 1); QVERIFY(result.exito); QCOMPARE(result.datos.toArray().size(), 1);
        QVERIFY(request.contains("Authorization: Bearer user-jwt"));
    }
    void invalidJsonFails() {
        response = "HTTP/1.1 200 OK\r\nContent-Length: 4\r\nConnection: close\r\n\r\noops";
        int calls=0; SupabaseClient::Resultado result;
        SupabaseClient::solicitar("GET", "/", {}, {}, this, [&](auto r) {result=r; ++calls;});
        QTRY_COMPARE(calls, 1); QVERIFY(!result.exito); QCOMPARE(result.codigo, 200);
    }
    void noContentSucceeds() {
        response = "HTTP/1.1 204 No Content\r\nConnection: close\r\n\r\n";
        int calls=0; SupabaseClient::Resultado result;
        SupabaseClient::solicitar("DELETE", "/", {}, {}, this, [&](auto r) {result=r; ++calls;});
        QTRY_COMPARE(calls, 1); QVERIFY(result.exito); QCOMPARE(result.codigo, 204);
    }
    void timeoutCallsBackOnce() {
        hold=true; qputenv("SNAKE_SUPABASE_TIMEOUT_MS", "40");
        int calls=0; SupabaseClient::Resultado result;
        SupabaseClient::solicitar("GET", "/", {}, {}, this, [&](auto r) {result=r; ++calls;});
        QTRY_COMPARE(calls, 1); QVERIFY(!result.exito); QVERIFY(result.mensaje.contains("tiempo")); QTest::qWait(80); QCOMPARE(calls, 1);
    }
    void deletedContextSuppressesCallback() {
        hold=true; qputenv("SNAKE_SUPABASE_TIMEOUT_MS", "40");
        int calls=0; auto *context = new QObject;
        SupabaseClient::solicitar("GET", "/", {}, {}, context, [&](auto) {++calls;}); delete context;
        QTest::qWait(120); QCOMPARE(calls, 0);
    }
    void httpErrorFailsAndRedactsSubmittedPassword() {
        const QByteArray body = "{\"message\":\"Rejected password secret123\"}";
        response = "HTTP/1.1 400 Bad Request\r\nContent-Length: " + QByteArray::number(body.size()) + "\r\nConnection: close\r\n\r\n" + body;
        int calls=0; SupabaseClient::Resultado result;
        SupabaseClient::solicitar("POST", "/", {{"password", "secret123"}}, {}, this, [&](auto r) {result=r; ++calls;});
        QTRY_COMPARE(calls, 1); QVERIFY(!result.exito); QCOMPARE(result.codigo, 400);
        QVERIFY(!result.mensaje.contains("secret123")); QVERIFY(result.mensaje.contains("Rejected password"));
    }
    void redirectIsNotFollowed() {
        QTcpServer destination; QVERIFY(destination.listen(QHostAddress::LocalHost));
        response = "HTTP/1.1 302 Found\r\nLocation: http://127.0.0.1:" + QByteArray::number(destination.serverPort()) + "/stolen\r\nContent-Length: 2\r\nConnection: close\r\n\r\n{}";
        int calls=0; SupabaseClient::Resultado result;
        SupabaseClient::solicitar("GET", "/", {}, "user-jwt", this, [&](auto r) {result=r; ++calls;});
        QTRY_COMPARE(calls, 1); QVERIFY(!result.exito); QCOMPARE(result.codigo, 302);
        QVERIFY(!destination.hasPendingConnections());
    }
    void errorsUseServerCodes() {
        QVERIFY(SupabaseClient::mensajeError(400, {{"error_code", "email_address_invalid"}}).contains("identificador"));
        QVERIFY(SupabaseClient::mensajeError(400, {{"code", "invalid_credentials"}}).contains("incorrect"));
        QCOMPARE(SupabaseClient::mensajeError(400, {{"message", "Specific server failure"}}), QString("Specific server failure"));
    }
};
QTEST_GUILESS_MAIN(TransportTest)
#include "test_supabaseclient.moc"

