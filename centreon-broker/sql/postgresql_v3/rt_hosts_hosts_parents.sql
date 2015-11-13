--
-- Hosts parenting relationships.
--
CREATE TABLE rt_hosts_hosts_parents (
  child_id int NOT NULL,
  parent_id int NOT NULL,

  UNIQUE (child_id, parent_id),
  INDEX (parent_id),
  FOREIGN KEY (child_id) REFERENCES rt_hosts (host_id)
    ON DELETE CASCADE,
  FOREIGN KEY (parent_id) REFERENCES rt_hosts (host_id)
    ON DELETE CASCADE
);
