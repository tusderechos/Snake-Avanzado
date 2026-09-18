-- Ejecutar manualmente como propietario en SQL Editor.
-- Cambia el identificador de acceso de las cuentas de Snake.
-- Conserva UUID, contraseña cifrada, progreso, skins y preferencias.
begin;
do $$ begin
 if exists (
   select 1 from auth.users u
   left join public.perfiles p on p.id=u.id
   where p.id is null or p.nombre_usuario is null
 ) then
   raise exception 'Hay cuentas sin perfil. Revisarlas antes de migrar.';
 end if;
end $$;
update auth.users u
set email=encode(sha256(convert_to(btrim(p.nombre_usuario),'UTF8')),'hex') || '@snake.invalid',
    email_confirmed_at=coalesce(u.email_confirmed_at,now()),
    updated_at=now()
from public.perfiles p
where p.id=u.id;
update auth.identities i
set identity_data=jsonb_set(
      jsonb_set(i.identity_data,'{email}',to_jsonb(u.email),true),
      '{email_verified}','true'::jsonb,true),
    updated_at=now()
from auth.users u join public.perfiles p on p.id=u.id
where i.user_id=u.id and i.provider='email';
commit;
select count(*) as cuentas_listas
from auth.users u join public.perfiles p on p.id=u.id
where u.email=encode(sha256(convert_to(btrim(p.nombre_usuario),'UTF8')),'hex') || '@snake.invalid';
