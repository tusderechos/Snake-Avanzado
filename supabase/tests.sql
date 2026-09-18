-- Ejecutar como postgres después de schema.sql. Todo se revierte al final.
begin;
insert into auth.users(id,raw_user_meta_data)
values ('8f9f7bda-4bcc-4670-9cb0-ea20b449d73c','{"nombre_usuario":"sql_test_snake"}');
select set_config('request.jwt.claim.sub','8f9f7bda-4bcc-4670-9cb0-ea20b449d73c',true);
set local role authenticated;
do $$ declare r jsonb; begin
 r := public.snake_perfil();
 if (r->>'puntos')::int<>0 then raise exception 'Perfil inicial incorrecto'; end if;
 if has_column_privilege(current_user,'public.perfiles','monedas','UPDATE') then raise exception 'Permiso inseguro monedas'; end if;
 if has_column_privilege(current_user,'public.perfiles','skins','UPDATE') then raise exception 'Permiso inseguro skins'; end if;
 if not has_column_privilege(current_user,'public.perfiles','volumen_musica','UPDATE') then raise exception 'Falta permiso preferencias'; end if;
 r := public.snake_partida('9344474c-6e80-4f2b-ac4e-0dd47afcc6d8',400,25);
 if (r->>'puntos')::int<>400 or (r->>'monedas')::int<>225 then raise exception 'Recompensa incorrecta'; end if;
 r := public.snake_partida('9344474c-6e80-4f2b-ac4e-0dd47afcc6d8',400,25);
 if (r->>'monedas')::int<>225 then raise exception 'Reintento duplicado'; end if;
 begin
  perform public.snake_partida('9344474c-6e80-4f2b-ac4e-0dd47afcc6d8',500,25);
  raise exception 'Aceptó reintento distinto';
 exception when invalid_parameter_value then null; end;
 r := public.snake_comprar_skin('gato');
 if (r->>'monedas')::int<>75 or r->>'skin_equipada'<>'gato' then raise exception 'Compra incorrecta'; end if;
 r := public.snake_comprar_skin('gato');
 if (r->>'monedas')::int<>75 then raise exception 'Cobró compra repetida'; end if;
 begin perform public.snake_comprar_skin('thanos'); raise exception 'Saldo insuficiente aceptado'; exception when invalid_parameter_value then null; end;
 begin perform public.snake_equipar_skin('dragon'); raise exception 'Equipó skin ajena'; exception when invalid_parameter_value then null; end;
 begin perform public.snake_nivel(3); raise exception 'Saltó niveles'; exception when invalid_parameter_value then null; end;
 perform public.snake_nivel(1);
 r := public.snake_nivel(0);
 if (r->>'nivel_historia')::int<>1 then raise exception 'Retrocedió nivel'; end if;
 r := public.snake_tutorial();
 if not (r->>'tutorial_completado')::boolean then raise exception 'No guardó tutorial'; end if;
 update public.perfiles set volumen_musica=0.4,control='WASD' where id=auth.uid();
 begin update public.perfiles set monedas=999999 where id=auth.uid(); raise exception 'Permitió editar monedas'; exception when insufficient_privilege then null; end;
end $$;
reset role;
select set_config('request.jwt.claim.sub','',true);
set local role anon;
do $$ begin
 perform * from public.ranking_publico limit 1;
 begin perform * from public.perfiles; raise exception 'Permitió leer perfiles anónimamente'; exception when insufficient_privilege then null; end;
 begin perform public.snake_perfil(); raise exception 'Permitió RPC anónimo'; exception when insufficient_privilege then null; end;
end $$;
reset role;
rollback;
