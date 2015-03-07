CREATE TABLE transacciones
(
  id serial NOT NULL,
  id_ussd integer NOT NULL,
  fecha_trx_client character varying(15) NOT NULL,
  fecha_trx_server character varying(15),
  estado integer, -- Identifica el estado en que se encuentra la transaccion integer actualmente
  CONSTRAINT transacciones_pkey PRIMARY KEY (id),
  CONSTRAINT transacciones_id_ussd_fkey FOREIGN KEY (id_ussd)
      REFERENCES ussd (id) MATCH SIMPLE
      ON UPDATE NO ACTION ON DELETE NO ACTION
)
WITH (
  OIDS=FALSE
);
ALTER TABLE transacciones
  OWNER TO postgres;
COMMENT ON COLUMN transacciones.estado IS 'Identifica el estado en que se encuentra la transaccion integer actualmente';

