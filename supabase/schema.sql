-- Ejecutar completo en el editor SQL de Supabase como postgres.
-- Es una migración repetible: conserva las filas existentes.
begin;
create table if not exists public.perfiles (
 id uuid primary key references auth.users(id) on delete cascade,
 nombre_usuario text unique not null,
 puntos integer not null default 0 check (puntos >= 0),
 monedas integer not null default 0 check (monedas >= 0),
 skin_equipada text not null default 'clasica',
 control text not null default 'FLECHAS' check (control in ('WASD','FLECHAS')),
 tutorial_completado boolean not null default false,
 nivel_historia integer not null default 0 check (nivel_historia between 0 and 3),
 creado_en timestamptz not null default now()
);
alter table public.perfiles add column if not exists skins text[] not null default array['clasica']::text[];
alter table public.perfiles add column if not exists volumen_musica real not null default 0.75 check (volumen_musica between 0 and 1);
alter table public.perfiles add column if not exists volumen_sonido real not null default 0.75 check (volumen_sonido between 0 and 1);
alter table public.perfiles add column if not exists actualizado_en timestamptz not null default now();
alter table public.perfiles add column if not exists avatar text not null default 'clasica';
do $$ begin
 if not exists (select 1 from pg_constraint where conname='perfiles_avatar_valido') then
  alter table public.perfiles add constraint perfiles_avatar_valido
   check (avatar in ('clasica','gato','dragon','burro','spiderman','miles','personaje','thanos'));
 end if;
end $$;
-- Preservar también la skin equipada antes de que existiera el inventario.
update public.perfiles set skins = array_append(skins, skin_equipada)
where not (skin_equipada = any(skins));

alter table public.perfiles enable row level security;
-- Eliminar todas las variantes antiguas, incluida cualquier lectura pública.
do $$ declare p record; c record; begin
 for p in select policyname from pg_policies where schemaname='public' and tablename='perfiles' loop
  execute format('drop policy %I on public.perfiles',p.policyname);
 end loop;
 -- REVOKE a nivel de tabla no elimina permisos históricos de columna.
 for c in select column_name from information_schema.columns where table_schema='public' and table_name='perfiles' loop
  execute format('revoke all privileges (%I) on public.perfiles from public, anon, authenticated',c.column_name);
 end loop;
end $$;
revoke all on public.perfiles from public, anon, authenticated;
grant select on public.perfiles to authenticated;
grant update (volumen_musica, volumen_sonido, control, avatar) on public.perfiles to authenticated;
create policy snake_leer_propio on public.perfiles for select to authenticated using (id=auth.uid());
create policy snake_preferencias_propias on public.perfiles for update to authenticated using (id=auth.uid()) with check (id=auth.uid());

create or replace function public.snake_marcar_actualizacion() returns trigger
language plpgsql set search_path = '' as $$ begin new.actualizado_en=now(); return new; end $$;
drop trigger if exists snake_actualizacion on public.perfiles;
create trigger snake_actualizacion before update on public.perfiles for each row execute function public.snake_marcar_actualizacion();

create or replace function public.crear_perfil_nuevo_usuario() returns trigger
language plpgsql security definer set search_path = '' as $$
declare n text := new.raw_user_meta_data ->> 'nombre_usuario';
begin
 if n is null or char_length(n) not between 3 and 15 or n !~ '^[[:alnum:]_]+$' then
  raise exception 'Nombre de usuario inválido: use de 3 a 15 letras, números o guion bajo.' using errcode='22023';
 end if;
 insert into public.perfiles(id,nombre_usuario) values(new.id,n);
 return new;
end $$;
drop trigger if exists crear_perfil_despues_registro on auth.users;
create trigger crear_perfil_despues_registro after insert on auth.users for each row execute function public.crear_perfil_nuevo_usuario();

create or replace function public.snake_perfil() returns jsonb
language plpgsql security definer set search_path = '' as $$
declare u uuid := auth.uid(); n text; r public.perfiles;
begin
 if u is null then raise exception 'Sesión requerida' using errcode='42501'; end if;
 select * into r from public.perfiles where id=u;
 if not found then
  select raw_user_meta_data ->> 'nombre_usuario' into n from auth.users where id=u;
  if n is null or char_length(n) not between 3 and 15 or n !~ '^[[:alnum:]_]+$' then
   raise exception 'La cuenta no tiene un nombre de usuario válido' using errcode='22023';
  end if;
  insert into public.perfiles(id,nombre_usuario) values(u,n) on conflict(id) do nothing;
  select * into strict r from public.perfiles where id=u;
 end if;
 return to_jsonb(r);
end $$;

create or replace function public.snake_comprar_skin(p_skin text) returns jsonb
language plpgsql security definer set search_path = '' as $$
declare u uuid := auth.uid(); r public.perfiles; precio integer;
begin
 if u is null then raise exception 'Sesión requerida' using errcode='42501'; end if;
 precio := case p_skin when 'clasica' then 0 when 'gato' then 150 when 'dragon' then 250 when 'burro' then 350 when 'spiderman' then 500 when 'miles' then 650 when 'personaje' then 850 when 'thanos' then 1400 end;
 if precio is null then raise exception 'Skin desconocida' using errcode='22023'; end if;
 perform public.snake_perfil();
 select * into strict r from public.perfiles where id=u for update;
 if p_skin='thanos' and r.nivel_historia < 3 then
  raise exception 'Complete los 3 niveles de Historia para desbloquear Thanos' using errcode='22023';
 end if;
 if not (p_skin=any(r.skins)) then
  if r.monedas < precio then raise exception 'Monedas insuficientes' using errcode='22023'; end if;
  update public.perfiles set monedas=monedas-precio,skins=array_append(skins,p_skin),skin_equipada=p_skin where id=u returning * into r;
 end if;
 return to_jsonb(r);
end $$;

create or replace function public.snake_equipar_skin(p_skin text) returns jsonb
language plpgsql security definer set search_path = '' as $$
declare u uuid := auth.uid(); r public.perfiles;
begin
 if u is null then raise exception 'Sesión requerida' using errcode='42501'; end if;
 perform public.snake_perfil();
 select * into strict r from public.perfiles where id=u for update;
 if p_skin is null or not (p_skin=any(r.skins)) then raise exception 'Skin no comprada' using errcode='22023'; end if;
 update public.perfiles set skin_equipada=p_skin where id=u returning * into r;
 return to_jsonb(r);
end $$;

create or replace function public.snake_tutorial() returns jsonb
language plpgsql security definer set search_path = '' as $$
declare u uuid := auth.uid(); r public.perfiles;
begin
 if u is null then raise exception 'Sesión requerida' using errcode='42501'; end if;
 perform public.snake_perfil();
 update public.perfiles set tutorial_completado=true where id=u returning * into r;
 return to_jsonb(r);
end $$;

create or replace function public.snake_nivel(p_nivel integer) returns jsonb
language plpgsql security definer set search_path = '' as $$
declare u uuid := auth.uid(); r public.perfiles;
begin
 if u is null then raise exception 'Sesión requerida' using errcode='42501'; end if;
 if p_nivel is null or p_nivel not between 0 and 3 then raise exception 'Nivel inválido' using errcode='22023'; end if;
 perform public.snake_perfil();
 select * into strict r from public.perfiles where id=u for update;
 if p_nivel > r.nivel_historia+1 then raise exception 'Complete el nivel anterior' using errcode='22023'; end if;
 update public.perfiles set nivel_historia=greatest(nivel_historia,p_nivel) where id=u returning * into r;
 return to_jsonb(r);
end $$;

create table if not exists public.snake_operaciones (
 usuario_id uuid not null references public.perfiles(id) on delete cascade,
 operacion uuid not null,
 puntos integer not null check(puntos between 0 and 1000000),
 bonus integer not null check(bonus between 0 and 175),
 creado_en timestamptz not null default now(),
 primary key(usuario_id,operacion)
);
alter table public.snake_operaciones enable row level security;
revoke all on public.snake_operaciones from public,anon,authenticated;
create or replace function public.snake_partida(p_operacion uuid,p_puntos integer,p_bonus integer) returns jsonb
language plpgsql security definer set search_path = '' as $$
declare u uuid := auth.uid(); r public.perfiles; anterior public.snake_operaciones;
begin
 if u is null then raise exception 'Sesión requerida' using errcode='42501'; end if;
 if p_operacion is null or p_puntos is null or p_bonus is null or p_puntos not between 0 and 1000000 or p_bonus not between 0 and 175 or p_bonus % 25 <> 0 then
  raise exception 'Resultado de partida inválido' using errcode='22023';
 end if;
 perform public.snake_perfil();
 select * into strict r from public.perfiles where id=u for update;
 select * into anterior from public.snake_operaciones where usuario_id=u and operacion=p_operacion;
 if found then
  if anterior.puntos<>p_puntos or anterior.bonus<>p_bonus then raise exception 'Operación repetida con datos distintos' using errcode='22023'; end if;
  return to_jsonb(r);
 end if;
 insert into public.snake_operaciones(usuario_id,operacion,puntos,bonus) values(u,p_operacion,p_puntos,p_bonus);
 update public.perfiles set puntos=puntos+p_puntos,monedas=monedas+p_puntos/2+p_bonus where id=u returning * into r;
 return to_jsonb(r);
end $$;

-- Vista del propietario: elude RLS deliberadamente, exponiendo solo datos del ranking.
create or replace view public.ranking_publico with (security_barrier=true,security_invoker=false) as
 select nombre_usuario,puntos,avatar from public.perfiles;
revoke all on public.ranking_publico from public,anon,authenticated;
grant select on public.ranking_publico to anon,authenticated;
revoke all on function public.crear_perfil_nuevo_usuario() from public,anon,authenticated;
revoke all on function public.snake_marcar_actualizacion() from public,anon,authenticated;
revoke all on function public.snake_perfil(),public.snake_comprar_skin(text),public.snake_equipar_skin(text),public.snake_tutorial(),public.snake_nivel(integer),public.snake_partida(uuid,integer,integer) from public,anon,authenticated;
grant execute on function public.snake_perfil(),public.snake_comprar_skin(text),public.snake_equipar_skin(text),public.snake_tutorial(),public.snake_nivel(integer),public.snake_partida(uuid,integer,integer) to authenticated;
commit;
