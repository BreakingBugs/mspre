
CREATE TABLE ussd
(
  clave character varying(41),
  id serial NOT NULL,
  CONSTRAINT ussd_pkey PRIMARY KEY (id)
)
WITH (
  OIDS=FALSE
);
ALTER TABLE ussd
  OWNER TO postgres;
