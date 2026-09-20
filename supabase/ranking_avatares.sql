-- Aplicar una vez en SQL Editor para mostrar el avatar público del ranking.
create or replace view public.ranking_publico
with (security_barrier=true, security_invoker=false) as
select nombre_usuario, puntos, avatar from public.perfiles;
revoke all on public.ranking_publico from public, anon, authenticated;
grant select on public.ranking_publico to anon, authenticated;
