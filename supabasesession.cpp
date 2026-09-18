#include "supabasesession.h"

SupabaseSession &SupabaseSession::instancia()
{
    static SupabaseSession sesion;
    return sesion;
}

void SupabaseSession::establecer(const QString &token, const QString &id,
                                 const QString &correo, const QString &nombreUsuario)
{
    m_token = token;
    m_id = id;
    m_correo = correo;
    m_nombreUsuario = nombreUsuario;
}

void SupabaseSession::limpiar()
{
    m_token.clear();
    m_id.clear();
    m_correo.clear();
    m_nombreUsuario.clear();
}

bool SupabaseSession::activa() const { return !m_token.isEmpty() && !m_id.isEmpty(); }
const QString &SupabaseSession::token() const { return m_token; }
const QString &SupabaseSession::id() const { return m_id; }
const QString &SupabaseSession::correo() const { return m_correo; }
const QString &SupabaseSession::nombreUsuario() const { return m_nombreUsuario; }
