# Convenience shims for running the project's code-style check via Docker.
# See Dockerfile.format and docker-compose.format.yml for the underlying setup.

.PHONY: format format-fix

# Diff-only check (matches CI; exit 2 means changes are needed).
format:
	docker compose -f docker-compose.format.yml run --rm format

# Apply formatting changes in place.
format-fix:
	docker compose -f docker-compose.format.yml run --rm format-apply
