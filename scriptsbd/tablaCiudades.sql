CREATE TABLE ciudades
(
  nombre character varying(30) NOT NULL,
  id_departamento integer NOT NULL,
  id serial NOT NULL,
  CONSTRAINT ciudades_pkey PRIMARY KEY (id),
  CONSTRAINT "ciudades_idDepartamento_fkey" FOREIGN KEY (id_departamento)
      REFERENCES departamentos (id) MATCH SIMPLE
      ON UPDATE NO ACTION ON DELETE NO ACTION
)
WITH (
  OIDS=FALSE
);
ALTER TABLE ciudades
  OWNER TO postgres;
