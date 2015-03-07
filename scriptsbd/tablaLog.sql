
CREATE TABLE log
(
  id serial NOT NULL,
  id_trx integer NOT NULL,
  fecha_cliente character varying(15) NOT NULL,
  fecha_servidor character varying(15) NOT NULL,
  entrada_salida smallint NOT NULL, -- 0 = Entrada...
  paquete character varying(4096),
  CONSTRAINT log_pkey PRIMARY KEY (id),
  CONSTRAINT log_id_transaccion_fkey FOREIGN KEY (id_trx)
      REFERENCES transacciones (id) MATCH SIMPLE
      ON UPDATE NO ACTION ON DELETE NO ACTION
)
WITH (
  OIDS=FALSE
);
ALTER TABLE log
  OWNER TO postgres;
COMMENT ON COLUMN log.entrada_salida IS '0 = Entrada
1 = Salida
(Otro valor no admitido)';

