CREATE TABLE departamentos
(
  nombre character varying(20) NOT NULL,
  id serial NOT NULL,
  CONSTRAINT departamentos_pkey PRIMARY KEY (id)
)
WITH (
  OIDS=FALSE
);
ALTER TABLE departamentos
  OWNER TO postgres;
