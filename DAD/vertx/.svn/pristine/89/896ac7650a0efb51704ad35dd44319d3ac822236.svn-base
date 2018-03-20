package vertx;

import java.util.HashMap;
import java.util.Map;

import io.vertx.core.AbstractVerticle;
import io.vertx.core.Future;
import io.vertx.core.json.Json;
import io.vertx.core.json.JsonObject;
import io.vertx.ext.asyncsql.MySQLClient;
import io.vertx.ext.sql.SQLClient;
import io.vertx.ext.web.Router;
import io.vertx.ext.web.RoutingContext;
import io.vertx.ext.web.handler.BodyHandler;

public class RestEP extends AbstractVerticle {

	// Prueba para la conexión al SQL.
	private Map<Integer, Usuario> databaseUsuario = new HashMap<>();
	private Map<String, Sensores> databaseSensores = new HashMap<>();
	private Map<Integer, RegFechas> databaseFechas = new HashMap<>();

	private SQLClient mySQLClient;

	public void start(Future<Void> startFuture) {
		createSomeData();
		// Se utiliza para el acceso a BD cuando corresponda.
		JsonObject mySQLClientConfig = new JsonObject().put("host", "127.0.0.1").put("port", 3306)
				.put("database", "tolmet").put("username", "root").put("password", "root");

		mySQLClient = MySQLClient.createShared(vertx, mySQLClientConfig);

		Router router = Router.router(vertx);
		// Inicializamos el Verticle. Lanzando el servidor HTTP
		vertx.createHttpServer().requestHandler(router::accept).listen(8083, res -> {
			if (res.succeeded()) {
				System.out.println("Servidor REST desplegado");
			} else {
				System.out.println("Error: " + res.cause());
			}
		});

		// Handlers para cada operación CRUD de usuarios.
		router.route("/api/usuarios").handler(BodyHandler.create());
		router.get("/api/usuarios").handler(this::getAllUsuarios);
		router.get("/api/usuarios/:idFilter").handler(this::getOneUsuario);
		router.put("/api/usuarios").handler(this::putElementUsuarios);
		router.delete("/api/usuarios/:idDelete").handler(this::deleteElementUsuarios);

		// Handlers para cada operación CRUD de sensores.
		router.route("/api/sensores").handler(BodyHandler.create());
		router.get("/api/sensores").handler(this::getAllSensores);
		router.get("/api/sensores/:idFilter").handler(this::getOneSensor);
		router.put("/api/sensores").handler(this::putElementSensores);
		router.delete("/api/sensores/:idDelete").handler(this::deleteElementSensores);

		// Handlers para cada operación CRUD de sensores.
		router.route("/api/fechas").handler(BodyHandler.create());
		router.get("/api/fechas").handler(this::getAllFechas);
		router.get("/api/fechas/:idFilter").handler(this::getOneFecha);
		router.put("/api/fechas").handler(this::putElementFechas);
		router.delete("/api/fechas/:idDelete").handler(this::deleteElementFechas);

	}

	// Operaciones CRUD de usuarios.
	private void getAllUsuarios(RoutingContext routingContext) {
		routingContext.response().putHeader("content-type", "application/json; charset=utf-8")
				.end(Json.encode(databaseUsuario.values()));
	}

	private void getOneUsuario(RoutingContext routingContext) {
		String paramStr = routingContext.request().getParam("idFilter");
		if (paramStr != null) {
			try {
				int param = Integer.parseInt(paramStr);
				/** Para cuando se utilice BD */
				// mySQLClient.getConnection(conn -> {
				// if (conn.succeeded()) {
				// SQLConnection connection = conn.result();
				// String query = "SELECT nombre, apellido, email, clave " +
				// "FROM usuario " + "WHERE id = ?";
				// JsonArray paramQuery = new JsonArray().add(param);
				// connection.queryWithParams(query, paramQuery, res -> {
				// if (res.succeeded()) {
				// routingContext.response().end(Json.encodePrettily(res.result().getRows()));
				// } else {
				// routingContext.response().setStatusCode(400).end("Error: " +
				// res.cause());
				// }
				// });
				// } else {
				// routingContext.response().setStatusCode(400).end("Error: " +
				// conn.cause());
				// }
				// });

				routingContext.response().setStatusCode(200).end(Json.encodePrettily(databaseUsuario.get(param)));
			} catch (ClassCastException e) {
				routingContext.response().setStatusCode(400).end();
			}
		} else {
			routingContext.response().setStatusCode(400).end();
		}
	}

	private void putElementUsuarios(RoutingContext routingContext) {
		Usuario state = Json.decodeValue(routingContext.getBodyAsString(), Usuario.class);
		databaseUsuario.put(state.getId(), state);
		routingContext.response().setStatusCode(201).end(Json.encode(state));
	}

	private void deleteElementUsuarios(RoutingContext routingContext) {
		String paramStr = routingContext.request().getParam("idDelete");
		if (paramStr != null) {
			try {
				int param = Integer.parseInt(paramStr);
				Usuario user = databaseUsuario.get(param);
				databaseUsuario.remove(param);
				routingContext.response().setStatusCode(200)
						.putHeader("content-type", "application/json; charset=utf-8")
						.end(Json.encodePrettily(user));
			} catch (ClassCastException e) {
				routingContext.response().setStatusCode(400).end();
			}
		} else {
			routingContext.response().setStatusCode(400).end();
		}
	}

	// Operaciones CRUD de sensores
	private void getAllSensores(RoutingContext routingContext) {
		routingContext.response().putHeader("content-type", "application/json; charset=utf-8")
				.end(Json.encode(databaseSensores.values()));
	}

	private void getOneSensor(RoutingContext routingContext) {
		String paramStr = routingContext.request().getParam("idFilter").trim();
		if (paramStr != null) {
			try {
				// mySQLClient.getConnection(conn -> {
				// if (conn.succeeded()) {
				// SQLConnection connection = conn.result();
				// String query = "SELECT idCodigo, tipoSensor, descripcion " +
				// "FROM sensores " + "WHERE id = ?";
				// JsonArray paramQuery = new JsonArray().add(param);
				// connection.queryWithParams(query, paramQuery, res -> {
				// if (res.succeeded()) {
				// routingContext.response().end(Json.encodePrettily(res.result().getRows()));
				// } else {
				// routingContext.response().setStatusCode(400).end("Error: " +
				// res.cause());
				// }
				// });
				// } else {
				// routingContext.response().setStatusCode(400).end("Error: " +
				// conn.cause());
				// }
				// });

				routingContext.response().setStatusCode(200).end(Json.encodePrettily(databaseSensores.get(paramStr)));
			} catch (ClassCastException e) {
				routingContext.response().setStatusCode(400).end();
			}
		} else {
			routingContext.response().setStatusCode(400).end();
		}
	}

	private void putElementSensores(RoutingContext routingContext) {
		Sensores state = Json.decodeValue(routingContext.getBodyAsString(), Sensores.class);
		databaseSensores.put(state.getIdCodigo(), state);
		routingContext.response().setStatusCode(201).end(Json.encode(state));
	}

	private void deleteElementSensores(RoutingContext routingContext) {
		String paramStr = routingContext.request().getParam("idDelete");
		if (paramStr != null) {
			try {
				Sensores sensor = databaseSensores.get(paramStr);
				databaseSensores.remove(paramStr);
				routingContext.response().setStatusCode(200)
						.putHeader("content-type", "application/json; charset=utf-8")
						.end(Json.encodePrettily(sensor));
			} catch (ClassCastException e) {
				routingContext.response().setStatusCode(400).end();
			}
		} else {
			routingContext.response().setStatusCode(400).end();
		}
	}

	// Operaciones CRUD de fechas
	private void getAllFechas(RoutingContext routingContext) {
		routingContext.response().putHeader("content-type", "application/json; charset=utf-8")
				.end(Json.encode(databaseFechas.values()));
	}

	private void getOneFecha(RoutingContext routingContext) {
		String paramStr = routingContext.request().getParam("idFilter");
		if (paramStr != null) {
			try {
				int param = Integer.parseInt(paramStr);

				// mySQLClient.getConnection(conn -> {
				// if (conn.succeeded()) {
				// SQLConnection connection = conn.result();
				// String query = "SELECT idSensor, tipoSensor, valor, fecha " +
				// "FROM regFechas "
				// + "WHERE id = ?";
				// JsonArray paramQuery = new JsonArray().add(param);
				// connection.queryWithParams(query, paramQuery, res -> {
				// if (res.succeeded()) {
				// routingContext.response().end(Json.encodePrettily(res.result().getRows()));
				// } else {
				// routingContext.response().setStatusCode(400).end("Error: " +
				// res.cause());
				// }
				// });
				// } else {
				// routingContext.response().setStatusCode(400).end("Error: " +
				// conn.cause());
				// }
				// });

				routingContext.response().setStatusCode(200).end(Json.encodePrettily(databaseFechas.get(param)));
			} catch (ClassCastException e) {
				routingContext.response().setStatusCode(400).end();
			}
		} else {
			routingContext.response().setStatusCode(400).end();
		}
	}

	private void putElementFechas(RoutingContext routingContext) {
		RegFechas state = Json.decodeValue(routingContext.getBodyAsString(), RegFechas.class);
		databaseFechas.put(state.getId(), state);
		routingContext.response().setStatusCode(201).end(Json.encode(state));
	}

	private void deleteElementFechas(RoutingContext routingContext) {
		String paramStr = routingContext.request().getParam("idDelete");
		if (paramStr != null) {
			try {
				int param = Integer.parseInt(paramStr);
				RegFechas fecha = databaseFechas.get(param);
				databaseUsuario.remove(param);
				routingContext.response().setStatusCode(200)
						.putHeader("content-type", "application/json; charset=utf-8")
						.end(Json.encodePrettily(fecha));
			} catch (ClassCastException e) {
				routingContext.response().setStatusCode(400).end();
			}
		} else {
			routingContext.response().setStatusCode(400).end();
		}
	}

	// Método para crear algunos datos
	private void createSomeData() {
		Usuario usuario1 = new Usuario("ALVARO", "MARTIN", "ALVARO@GMAIL.COM", "CLAVE");
		Usuario usuario2 = new Usuario("JUANMA", "PEREZ", "JUANMA@GMAIL.COM", "CLAVE");
		databaseUsuario.put(usuario1.getId(), usuario1);
		databaseUsuario.put(usuario2.getId(), usuario2);
		Sensores sensor1 = new Sensores("ST1", "TEMP", "Sensor salón.");
		Sensores sensor2 = new Sensores("SH1", "HUM", "Sensor patio");
		databaseSensores.put(sensor1.getIdCodigo(), sensor1);
		databaseSensores.put(sensor2.getIdCodigo(), sensor2);
		RegFechas fecha1 = new RegFechas("ST1", "TEMP", "20,0");
		databaseFechas.put(fecha1.getId(), fecha1);
	}

}
