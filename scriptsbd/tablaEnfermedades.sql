CREATE TABLE enfermedades
(
  nombre character varying(50) NOT NULL,
  id serial NOT NULL,
  CONSTRAINT enfermedades_pkey PRIMARY KEY (id)
)
WITH (
  OIDS=FALSE
);
ALTER TABLE enfermedades
  OWNER TO postgres;
