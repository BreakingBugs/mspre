
CREATE TABLE enfermedades_registro
(
  id serial NOT NULL,
  id_ciudad integer NOT NULL,
  id_enfermedad integer NOT NULL,
  id_trx integer NOT NULL,
  CONSTRAINT enfermedadesRegistro_pkey PRIMARY KEY (id),
  CONSTRAINT enfermedadesRegistro_id_ciudad_fkey FOREIGN KEY (id_ciudad)
      REFERENCES ciudades (id) MATCH SIMPLE
      ON UPDATE NO ACTION ON DELETE NO ACTION,
  CONSTRAINT enfermedadesRegistro_id_enfermedad_fkey FOREIGN KEY (id_enfermedad)
      REFERENCES enfermedades (id) MATCH SIMPLE
      ON UPDATE NO ACTION ON DELETE NO ACTION,
  CONSTRAINT enfermedadesRegistro_id_transaccion_fkey FOREIGN KEY (id_trx)
      REFERENCES transacciones (id) MATCH SIMPLE
      ON UPDATE NO ACTION ON DELETE NO ACTION
)
WITH (
  OIDS=FALSE
);
ALTER TABLE enfermedades_registro
  OWNER TO postgres;
