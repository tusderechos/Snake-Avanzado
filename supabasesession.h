#ifndef SUPABASESESSION_H
#define SUPABASESESSION_H

#include <QString>

class SupabaseSession
{
public:
    static SupabaseSession &instancia();
    void establecer(const QString &token, const QString &id, const QString &correo,
                    const QString &nombreUsuario);
    void limpiar();
    bool activa() const;
    const QString &token() const;
    const QString &id() const;
    const QString &correo() const;
    const QString &nombreUsuario() const;

private:
    SupabaseSession() = default;
    QString m_token;
    QString m_id;
    QString m_correo;
    QString m_nombreUsuario;
};

#endif
