-- Create the project table
CREATE TABLE IF NOT EXISTS project (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    version TEXT NOT NULL,
    used_in_production INTEGER
);

-- Create the dependency table with a foreign key to the project table
CREATE TABLE IF NOT EXISTS dependency (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    version TEXT NOT NULL,
    project_id INTEGER,
    FOREIGN KEY (project_id) REFERENCES project(id)
);

-- Query to insert a project
INSERT INTO project (name, version, used_in_production) VALUES (?, ?, 0);

-- Query to insert or update a dependency linked to a project
INSERT OR REPLACE INTO dependency (name, version, project_id) VALUES (?, ?, ?);

-- Query to select all dependencies linked to a specific project
SELECT * FROM dependency WHERE project_id = ?;

-- Query to remove a dependency
DELETE FROM dependency WHERE name = ? AND project_id = ?;

-- Query to remove a project and its associated dependencies
DELETE FROM project WHERE id = ?;
DELETE FROM dependency WHERE project_id = ?;

