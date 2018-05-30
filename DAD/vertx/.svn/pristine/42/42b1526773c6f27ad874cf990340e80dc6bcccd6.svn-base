package vertx;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;
import java.util.stream.Stream;
import org.quartz.CronScheduleBuilder;
import org.quartz.CronTrigger;
import org.quartz.JobBuilder;
import org.quartz.JobDetail;
import org.quartz.SchedulerException;
import org.quartz.SchedulerFactory;
import org.quartz.TriggerBuilder;
import com.google.common.collect.HashMultimap;
import com.google.common.collect.Multimap;
import io.netty.handler.codec.mqtt.MqttQoS;
import io.vertx.core.AbstractVerticle;
import io.vertx.core.Future;
import io.vertx.core.buffer.Buffer;
import io.vertx.core.json.Json;
import io.vertx.core.json.JsonArray;
import io.vertx.core.json.JsonObject;
import io.vertx.ext.asyncsql.MySQLClient;
import io.vertx.ext.sql.SQLClient;
import io.vertx.ext.sql.SQLConnection;
import io.vertx.ext.web.Router;
import io.vertx.ext.web.RoutingContext;
import io.vertx.ext.web.handler.BodyHandler;
import io.vertx.ext.web.handler.StaticHandler;
import io.vertx.mqtt.MqttClient;
import io.vertx.mqtt.MqttClientOptions;
import io.vertx.mqtt.MqttEndpoint;
import io.vertx.mqtt.MqttServer;
import io.vertx.mqtt.MqttTopicSubscription;
import io.vertx.mqtt.messages.MqttPublishMessage;

public class RestEP extends AbstractVerticle {
	// VARIABLES GLOBALES
	private static SQLClient mySQLClient;
	private static MqttClient mqttClient;
	private static Multimap<String, MqttEndpoint> clientTopics;
	private static String elementoAutomatico;
	private static String horaElementoAutomatico;
	private static List<JobDetail> jobs = new ArrayList<>();

	public void start(Future<Void> startFuture) {
		// Publicación de mensajes en el topic.
		clientTopics = HashMultimap.create();
		// Acceso a BASE DE DATOS
		JsonObject mySQLClientConfig = new JsonObject().put("host", "127.0.0.1").put("port", 3306)
				.put("database", "tolmet").put("username", "root").put("password", "root");
		mySQLClient = MySQLClient.createShared(vertx, mySQLClientConfig);

		// Inicializamos el Verticle. Lanzando el servidor HTTP
		Router router = Router.router(vertx);

		// Para crear el index.html
		router.route("/inicio/*").handler(StaticHandler.create("inicio"));

		vertx.createHttpServer().requestHandler(router::accept).listen(8083, res -> {
			if (res.succeeded()) {
				System.out.println("Servidor REST desplegado");
			} else {
				System.out.println("Error: " + res.cause());
			}
		});

		// Creación del servidor MQTT
		MqttServer mqttServer = MqttServer.create(vertx);
		mqttServer.endpointHandler(endpoint -> {
			System.out.println("Nuevo cliente MQTT [" + endpoint.clientIdentifier()
					+ "] solicitando suscribirse [Nueva sesión: " + endpoint.isCleanSession() + "]");
			endpoint.accept(false);
			handleSubscription(endpoint);
			handleUnsubscription(endpoint);
			publishHandler(endpoint);
			handleClientDisconnect(endpoint);
		}).listen(ar -> {
			if (ar.succeeded()) {
				System.out.println("MQTT server en puerto " + ar.result().actualPort());
			} else {
				System.out.println("Error desplegando el MQTT server");
				ar.cause().printStackTrace();
			}
		});

		// CreamosCliente principal, para el intercambio de mensajes
		MqttClientOptions opciones = new MqttClientOptions();
		opciones.setClientId("clientePrincipal");
		opciones.setAutoKeepAlive(true);
		mqttClient = MqttClient.create(vertx, opciones);
		mqttClient.connect(1883, "localhost", s -> {
		});

		// Realización de tareas periódicas.(Añadir datos a BD)
		final Runnable tarea = new Runnable() {
			public void run() {
				mySQLClient.getConnection(conn -> {
					if (conn.succeeded()) {
						SQLConnection connection = conn.result();
						String query = "SELECT idWifi " + "FROM modwifi ";
						connection.query(query, res -> {
							if (res.succeeded()) {
								List<JsonObject> objects = res.result().getRows();
								for (JsonObject j : objects) {
									String aux = "in" + j.getString("idWifi");
									mqttClient.publish(aux, Buffer.buffer("addDatos"), MqttQoS.AT_LEAST_ONCE, false,
											false);
								}
							} else {
								System.out.println("No se pudo realizar la tarea periodica.");
							}
						});
					} else {
						System.out.println("Conexion fallida.");
					}
				});
			}
		};
		ScheduledExecutorService timer = Executors.newSingleThreadScheduledExecutor();
		timer.scheduleAtFixedRate(tarea, 1, 8, TimeUnit.HOURS);

		// Handlers para cada operación CRUD abrir/cerrar persianas.
		router.route("/api/abrirPersianas").handler(BodyHandler.create());
		router.get("/api/abrirPersianas/abrirPersiana/:idMensaje").handler(this::abrirPersiana);
		router.route("/api/cerrarPersianas").handler(BodyHandler.create());
		router.get("/api/cerrarPersianas/cerrarPersiana/:idMensaje").handler(this::cerrarPersiana);

		// Handlers para cada operación CRUD desplegar/recoger toldos.
		router.route("/api/desplegarToldos").handler(BodyHandler.create());
		router.get("/api/desplegarToldos/desplegarToldo/:idMensaje").handler(this::desplegarToldo);
		router.route("/api/recogerToldos").handler(BodyHandler.create());
		router.get("/api/recogerToldos/recogerToldo/:idMensaje").handler(this::recogerToldo);

		// Handlers para cada operación CRUD obtencion de datos
		router.route("/api/obtenerDatos").handler(BodyHandler.create());
		router.put("/api/obtenerDatos").handler(this::obtenerDatos);
		// Handlers para cada operación CRUD obtencion de datos
		router.route("/api/automatico").handler(BodyHandler.create());
		router.put("/api/automatico").handler(this::putElemento);

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

		// Handlers para cada operación CRUD de fechas.
		router.route("/api/fechas").handler(BodyHandler.create());
		router.get("/api/fechas").handler(this::getAllFechas);
		router.get("/api/fechas/:idFilter").handler(this::getOneFecha);
		router.put("/api/fechas").handler(this::putElementFechas);
		router.delete("/api/fechas/:idDelete").handler(this::deleteElementFechas);

		// Handlers para cada operación CRUD de ModWifi.
		router.route("/api/wifi").handler(BodyHandler.create());
		router.get("/api/wifi").handler(this::getAllMods);
		router.get("/api/wifi/:idFilter").handler(this::getOneMod);
		router.put("/api/wifi").handler(this::putElementMod);
		router.delete("/api/wifi/:idDelete").handler(this::deleteElementMod);

		// Handlers para cada operación CRUD de Elementos.
		router.route("/api/elementos").handler(BodyHandler.create());
		router.get("/api/elementos").handler(this::getAllElem);
		router.get("/api/elementos/:idFilter").handler(this::getOneElem);
		router.put("/api/elementos").handler(this::putElem);
		router.delete("/api/elementos/:idDelete").handler(this::deleteElem);
		router.get("/api/elementos/estado/:idFilter").handler(this::getState);
		router.route("/api/elementos/updateEstado").handler(BodyHandler.create());
		router.put("/api/elementos/updateEstado").handler(this::updateState);
	}

	/********************************************************************************************
	 * APLICACIÓN * *
	 *******************************************************************************************/
	/**
	 * Indica el elemento seleecionado, y la hora para abrir las persianas
	 * automaticamente
	 * 
	 * @param routingContext
	 */
	private void putElemento(RoutingContext routingContext) {
		JsonObject j = routingContext.getBodyAsJson();
		elementoAutomatico = j.getString("elemento");
		horaElementoAutomatico = j.getString("hora");
		levantamientoAutomatico(elementoAutomatico, horaElementoAutomatico);
	}

	public static String elementoAutomatico() {
		return elementoAutomatico;
	}

	private void levantamientoAutomatico(String elemento, String hora) {
		String horaFinal = "";
		if (!elemento.isEmpty()) {
			if (!hora.isEmpty()) {
				String[] lista = hora.split(":");
				horaFinal = "0 " + lista[1] + " " + lista[0] + " * * ?";
			} else { horaFinal = "0 0 0 * * ?";	}
			JobDetail job = null;
			if (jobs.isEmpty()) {
				job = JobBuilder.newJob(TareaLevantarPersiana.class).withDescription(elemento).build();
				jobs.add(job);
			} else { for (JobDetail j : jobs) {
					 if (j.getDescription().equals(elemento)) {
						job = j;
						break;
				    	} else {
					    	job = JobBuilder.newJob(TareaLevantarPersiana.class).withDescription(elemento).build();
						    jobs.add(job);
						    break;
					    }
				    }
			   }
			CronTrigger trigger = TriggerBuilder.newTrigger().withSchedule
					(CronScheduleBuilder.cronSchedule(horaFinal)).build();
			SchedulerFactory schedFact = new org.quartz.impl.StdSchedulerFactory();
			org.quartz.Scheduler sched = null;
			try {
				sched = schedFact.getScheduler();
				sched.start();
				sched.deleteJob(job.getKey());
				sched.scheduleJob(job, trigger);
				if (hora.isEmpty()) {
					sched.pauseJob(job.getKey());
				}
			} catch (SchedulerException e1) {
				e1.printStackTrace();
			}
		}
	}

	/**
	 * Obtener datos de un sensor específico.
	 * 
	 * @param routingContext
	 */
	private void obtenerDatos(RoutingContext routingContext) {
		JsonObject j = routingContext.getBodyAsJson();
		if (!j.getString("sensor").isEmpty() && !j.getString("fDesde").isEmpty() && !j.getString("fHasta").isEmpty()) {
			String fechaDesde = parseoFecha(j.getString("fDesde"));
			String fechaHasta = parseoFecha(j.getString("fHasta"));
			mySQLClient.getConnection(conn -> {
				if (conn.succeeded()) {
					SQLConnection connection = conn.result();
					String query = "SELECT valor, fecha FROM regfechas " + "WHERE idSensor = '" + j.getString("sensor")
							+ "' AND fecha BETWEEN '" + fechaDesde + "' AND '" + fechaHasta + "'";
					connection.query(query, res -> {
						if (res.succeeded()) {
							routingContext.response().end(Json.encodePrettily(res.result().getRows()));
						} else {
							routingContext.response().setStatusCode(400).end("Error: " + res.cause());
						}
					});
				} else {
					routingContext.response().setStatusCode(400).end("Error: " + conn.cause());
				}
			});
		} else {
			routingContext.response().setStatusCode(400).end("Error");
		}
	}

	private String parseoFecha(String fecha) {
		String res = "";
		String[] lista = fecha.split("-");
		res += lista[2] + "/";
		res += lista[1] + "/";
		res += lista[0] + "*";
		return res;
	}

	/**
	 * Indicamos nombre de persiana, y envía mensaje a nodeMCU específico.
	 * 
	 * @param routingContext
	 */
	private void cerrarPersiana(RoutingContext routingContext) {
		String paramStr = routingContext.request().getParam("idMensaje").trim();
		if (paramStr != null) {
			try {
				mySQLClient.getConnection(conn -> {
					if (conn.succeeded()) {
						SQLConnection connection = conn.result();
						String query = "SELECT idElemento, idWifiElem " + "FROM elementos " + "WHERE idElemento = ?";
						JsonArray paramQuery = new JsonArray().add(paramStr);
						connection.queryWithParams(query, paramQuery, res -> {
							if (res.succeeded()) {
								JsonObject objeto = res.result().getRows().get(0);
								String nodeMCU = "in" + objeto.getValue("idWifiElem").toString();
								mqttClient.publish(nodeMCU, Buffer.buffer("cerrarPersiana"), MqttQoS.AT_LEAST_ONCE,
										false, false);
								routingContext.response().end("Mensaje, cerrarPersiana enviado al topic = " + nodeMCU);
							} else {
								routingContext.response().end("Error");
							}
						});
					}
				});
			} catch (ClassCastException e) {
				routingContext.response().end("Error2");
			}
		} else {
			routingContext.response().end("Error3");
		}
	}

	/**
	 * Indicamos nombre de persiana, y envía mensaje a nodeMCU específico.
	 * 
	 * @param routingContext
	 * @throws InterruptedException
	 */
	private void abrirPersiana(RoutingContext routingContext) {
		String paramStr = routingContext.request().getParam("idMensaje").trim();
		if (paramStr != null) {
			try {
				mySQLClient.getConnection(conn -> {
					if (conn.succeeded()) {
						SQLConnection connection = conn.result();
						String query = "SELECT idElemento, idWifiElem " + "FROM elementos " + "WHERE idElemento = ?";
						JsonArray paramQuery = new JsonArray().add(paramStr);
						connection.queryWithParams(query, paramQuery, res -> {
							if (res.succeeded()) {
								JsonObject objeto = res.result().getRows().get(0);
								String nodeMCU = "in" + objeto.getValue("idWifiElem").toString();
								mqttClient.publish(nodeMCU, Buffer.buffer("abrirPersiana"), MqttQoS.AT_LEAST_ONCE,
										false, false);
								routingContext.response().end("Mensaje, abrirPersiana enviado al topic = " + nodeMCU);
							} else {
								routingContext.response().end("Error");
							}
						});
					}
				});
			} catch (ClassCastException e) {
				routingContext.response().end("Error2");
			}
		} else {
			routingContext.response().end("Error3");
		}
	}

	/**
	 * Uso automático de levantamiento de persiana.
	 * 
	 * @param paramStr
	 */
	public static void abrirPersianaLuminosidad(String paramStr) {
		if (paramStr != null) {
			try {
				mySQLClient.getConnection(conn -> {
					if (conn.succeeded()) {
						SQLConnection connection = conn.result();
						String query = "SELECT idElemento, idWifiElem " + "FROM elementos " + "WHERE idElemento = ?";
						JsonArray paramQuery = new JsonArray().add(paramStr);
						connection.queryWithParams(query, paramQuery, res -> {
							if (res.succeeded()) {
								JsonObject objeto = res.result().getRows().get(0);
								String nodeMCU = "in" + objeto.getValue("idWifiElem").toString();
								mqttClient.publish(nodeMCU, Buffer.buffer("abrirPersiana"), MqttQoS.AT_LEAST_ONCE,
										false, false);
							}
						});
					}
				});
			} catch (ClassCastException e) {

			}
		} else {

		}
	}

	/**
	 * Indicamos nombre de toldo, y envía mensaje a nodeMCU específico.
	 * 
	 * @param routingContext
	 */
	private void recogerToldo(RoutingContext routingContext) {
		String paramStr = routingContext.request().getParam("idMensaje").trim();
		if (paramStr != null) {
			try {
				mySQLClient.getConnection(conn -> {
					if (conn.succeeded()) {
						SQLConnection connection = conn.result();
						String query = "SELECT idElemento, idWifiElem " + "FROM elementos " + "WHERE idElemento = ?";
						JsonArray paramQuery = new JsonArray().add(paramStr);
						connection.queryWithParams(query, paramQuery, res -> {
							if (res.succeeded()) {
								JsonObject objeto = res.result().getRows().get(0);
								String nodeMCU = "in" + objeto.getValue("idWifiElem").toString();
								mqttClient.publish(nodeMCU, Buffer.buffer("recogerToldo"), MqttQoS.AT_LEAST_ONCE, false,
										false);
								routingContext.response().end("Mensaje, recogerToldo enviado al topic = " + nodeMCU);
							} else {
								routingContext.response().end("Error");
							}
						});
					}
				});
			} catch (ClassCastException e) {
				routingContext.response().end("Error2");
			}
		} else {
			routingContext.response().end("Error3");
		}
	}

	/**
	 * Indicamos nombre del toldo, y envía mensaje a nodeMCU específico.
	 * 
	 * @param routingContext
	 * @throws InterruptedException
	 */
	private void desplegarToldo(RoutingContext routingContext) {
		String paramStr = routingContext.request().getParam("idMensaje").trim();
		if (paramStr != null) {
			try {
				mySQLClient.getConnection(conn -> {
					if (conn.succeeded()) {
						SQLConnection connection = conn.result();
						String query = "SELECT idElemento, idWifiElem " + "FROM elementos " + "WHERE idElemento = ?";
						JsonArray paramQuery = new JsonArray().add(paramStr);
						connection.queryWithParams(query, paramQuery, res -> {
							if (res.succeeded()) {
								JsonObject objeto = res.result().getRows().get(0);
								String nodeMCU = "in" + objeto.getValue("idWifiElem").toString();
								mqttClient.publish(nodeMCU, Buffer.buffer("desplegarToldo"), MqttQoS.AT_LEAST_ONCE,
										false, false);
								routingContext.response().end("Mensaje, desplegarToldo enviado al topic = " + nodeMCU);
							} else {
								routingContext.response().end("Error");
							}
						});
					}
				});
			} catch (ClassCastException e) {
				routingContext.response().end("Error2");
			}
		} else {
			routingContext.response().end("Error3");
		}
	}

	/********************************************************************************************
	 * Métodos para MQTT * *
	 *******************************************************************************************/
	// HANDLERS para la subscripción
	private static void handleSubscription(MqttEndpoint endpoint) {
		endpoint.subscribeHandler(subscribe -> {
			List<MqttQoS> grantedQosLevels = new ArrayList<>();
			for (MqttTopicSubscription s : subscribe.topicSubscriptions()) {
				System.out.println("Suscripción al topic " + s.topicName());
				clientTopics.put(s.topicName(), endpoint);
				grantedQosLevels.add(s.qualityOfService());
			}
			endpoint.subscribeAcknowledge(subscribe.messageId(), grantedQosLevels);
		});
	}

	// HADLERS para la desubscripción
	private static void handleUnsubscription(MqttEndpoint endpoint) {
		endpoint.unsubscribeHandler(unsubscribe -> {
			for (String t : unsubscribe.topics()) {
				clientTopics.remove(t, endpoint);
				System.out.println(
						"El cliente " + endpoint.clientIdentifier() + " ha eliminado la suscripción al canal " + t);
			}
			endpoint.unsubscribeAcknowledge(unsubscribe.messageId());
		});
	}

	// HADLERS para la publicación de mensajes
	private static void publishHandler(MqttEndpoint endpoint) {
		endpoint.publishHandler(message -> {
			handleMessage(message, endpoint);
		}).publishReleaseHandler(messageId -> {
			endpoint.publishComplete(messageId);
		});
	}

	private static void handleMessage(MqttPublishMessage message, MqttEndpoint endpoint) {
		System.out.println("Mensaje publicado");
		System.out.println("Topic: " + message.topicName() + ". Contenido: " + message.payload().toString());
		System.out.println("Origen: " + endpoint.clientIdentifier());
		for (MqttEndpoint client : clientTopics.get(message.topicName())) {
			System.out.println("Destino: " + client.clientIdentifier());
			if (!client.clientIdentifier().equals(endpoint.clientIdentifier()))
				client.publish(message.topicName(), message.payload(), message.qosLevel(), message.isDup(),
						message.isRetain());
		}
		if (message.qosLevel() == MqttQoS.AT_LEAST_ONCE) {
			String topicName = message.topicName();
			switch (topicName) {
			// Hacer algo con el mensaje si es necesario
			// TODO
			}
			endpoint.publishAcknowledge(message.messageId());
		} else if (message.qosLevel() == MqttQoS.EXACTLY_ONCE) {
			endpoint.publishRelease(message.messageId());
		}
	}

	protected void handleClientDisconnect(MqttEndpoint endpoint) {
		endpoint.disconnectHandler(disconnect -> {
			Stream.of(clientTopics.keySet()).filter(e -> clientTopics.containsEntry(e, endpoint))
					.forEach(s -> clientTopics.remove(s, endpoint));
			System.out.println("El cliente remoto se ha desconectado [" + endpoint.clientIdentifier() + "]");
		});

	}

	/********************************************************************************************
	 * Métodos para VERT.X * *
	 *******************************************************************************************/
	// Operaciones CRUD de usuarios.
	private void getAllUsuarios(RoutingContext routingContext) {
		/** Para cuando se utilice BD */
		mySQLClient.getConnection(conn -> {
			if (conn.succeeded()) {
				SQLConnection connection = conn.result();
				String query = "SELECT * " + "FROM usuario ";
				connection.query(query, res -> {
					if (res.succeeded()) {
						routingContext.response().end(Json.encodePrettily(res.result().getRows()));
					} else {
						routingContext.response().setStatusCode(400).end("Error: " + res.cause());
					}
				});
			} else {
				routingContext.response().setStatusCode(400).end("Error: " + conn.cause());
			}
		});
	}

	private void getOneUsuario(RoutingContext routingContext) {
		String paramStr = routingContext.request().getParam("idFilter");
		if (paramStr != null) {
			try {
				int param = Integer.parseInt(paramStr);
				/** Para cuando se utilice BD */
				mySQLClient.getConnection(conn -> {
					if (conn.succeeded()) {
						SQLConnection connection = conn.result();
						String query = "SELECT id, nombre, apellido, email, clave " + "FROM usuario " + "WHERE id = ?";
						JsonArray paramQuery = new JsonArray().add(param);
						connection.queryWithParams(query, paramQuery, res -> {
							if (res.succeeded()) {
								routingContext.response().end(Json.encodePrettily(res.result().getRows().get(0)));
							} else {
								routingContext.response().setStatusCode(400).end("Error: " + res.cause());
							}
						});
					} else {
						routingContext.response().setStatusCode(400).end("Error: " + conn.cause());
					}
				});
			} catch (ClassCastException e) {
				routingContext.response().setStatusCode(400).end();
			}
		} else {
			routingContext.response().setStatusCode(400).end();
		}
	}

	private void putElementUsuarios(RoutingContext routingContext) {
		Usuario state = Json.decodeValue(routingContext.getBodyAsString(), Usuario.class);
		/** Para cuando se utilice BD */
		mySQLClient.getConnection(conn -> {
			if (conn.succeeded()) {
				SQLConnection connection = conn.result();
				String query = "INSERT INTO usuario (nombre, apellido, email, clave) VALUES ('" + state.getNombre()
						+ "','" + state.getApellido() + "','" + state.getEmail() + "','" + state.getclave() + "')";
				connection.query(query, res -> {
					if (res.succeeded()) {
						routingContext.response().end(Json.encodePrettily(res.result().getRows()));
					} else {
						routingContext.response().setStatusCode(400).end("Error: " + res.cause());
					}
				});
			} else {
				routingContext.response().setStatusCode(400).end("Error: " + conn.cause());
			}
		});
	}

	private void deleteElementUsuarios(RoutingContext routingContext) {
		String paramStr = routingContext.request().getParam("idDelete");
		if (paramStr != null) {
			try {
				int param = Integer.parseInt(paramStr);
				/** Para cuando se utilice BD */
				mySQLClient.getConnection(conn -> {
					if (conn.succeeded()) {
						SQLConnection connection = conn.result();
						String query = "DELETE FROM usuario " + "WHERE id = ?";
						JsonArray paramQuery = new JsonArray().add(param);
						connection.queryWithParams(query, paramQuery, res -> {
							if (res.succeeded()) {
								routingContext.response().end(Json.encodePrettily(res.result().getRows()));
							} else {
								routingContext.response().setStatusCode(400).end("Error: " + res.cause());
							}
						});
					} else {
						routingContext.response().setStatusCode(400).end("Error: " + conn.cause());
					}
				});
			} catch (ClassCastException e) {
				routingContext.response().setStatusCode(400).end();
			}
		} else {
			routingContext.response().setStatusCode(400).end();
		}
	}

	// Operaciones CRUD de sensores
	private void getAllSensores(RoutingContext routingContext) {
		mySQLClient.getConnection(conn -> {
			if (conn.succeeded()) {
				SQLConnection connection = conn.result();
				String query = "SELECT * " + "FROM sensores ";
				connection.query(query, res -> {
					if (res.succeeded()) {
						routingContext.response().end(Json.encodePrettily(res.result().getRows()));
					} else {
						routingContext.response().setStatusCode(400).end("Error: " + res.cause());
					}
				});
			} else {
				routingContext.response().setStatusCode(400).end("Error: " + conn.cause());
			}
		});
	}

	private void getOneSensor(RoutingContext routingContext) {
		String paramStr = routingContext.request().getParam("idFilter").trim();
		if (paramStr != null) {
			try {
				mySQLClient.getConnection(conn -> {
					if (conn.succeeded()) {
						SQLConnection connection = conn.result();
						String query = "SELECT idSensor, tipoSensor, descripcion, idWifi " + "FROM sensores "
								+ "WHERE idSensor = ?";
						JsonArray paramQuery = new JsonArray().add(paramStr);
						connection.queryWithParams(query, paramQuery, res -> {
							if (res.succeeded()) {
								routingContext.response().end(Json.encodePrettily(res.result().getRows().get(0)));
							} else {
								routingContext.response().setStatusCode(400).end("Error: " + res.cause());
							}
						});
					} else {
						routingContext.response().setStatusCode(400).end("Error: " + conn.cause());
					}
				});
			} catch (ClassCastException e) {
				routingContext.response().setStatusCode(400).end();
			}
		} else {
			routingContext.response().setStatusCode(400).end();
		}
	}

	private void putElementSensores(RoutingContext routingContext) {
		Sensores state = Json.decodeValue(routingContext.getBodyAsString(), Sensores.class);
		mySQLClient.getConnection(conn -> {
			if (conn.succeeded()) {
				SQLConnection connection = conn.result();
				String query = "INSERT INTO sensores (idSensor, tipoSensor, descripcion, idWifi) VALUES ('"
						+ state.getIdSensor() + "','" + state.getTipoSensor() + "','" + state.getDescripcion() + "','"
						+ state.getIdWifi() + "')";
				connection.query(query, res -> {
					if (res.succeeded()) {
						routingContext.response().end(Json.encodePrettily(res.result().getRows()));
					} else {
						routingContext.response().setStatusCode(400).end("Error: " + res.cause());
					}
				});
			} else {
				routingContext.response().setStatusCode(400).end("Error: " + conn.cause());
			}
		});
	}

	private void deleteElementSensores(RoutingContext routingContext) {
		String paramStr = routingContext.request().getParam("idDelete");
		if (paramStr != null) {
			try {
				mySQLClient.getConnection(conn -> {
					if (conn.succeeded()) {
						SQLConnection connection = conn.result();
						String query = "DELETE FROM sensores " + "WHERE idSensor = ?";
						JsonArray paramQuery = new JsonArray().add(paramStr);
						connection.queryWithParams(query, paramQuery, res -> {
							if (res.succeeded()) {
								routingContext.response().end(Json.encodePrettily(res.result().getRows()));
							} else {
								routingContext.response().setStatusCode(400).end("Error: " + res.cause());
							}
						});
					} else {
						routingContext.response().setStatusCode(400).end("Error: " + conn.cause());
					}
				});
			} catch (ClassCastException e) {
				routingContext.response().setStatusCode(400).end();
			}
		} else {
			routingContext.response().setStatusCode(400).end();
		}
	}

	// Operaciones CRUD de fechas
	private void getAllFechas(RoutingContext routingContext) {
		mySQLClient.getConnection(conn -> {
			if (conn.succeeded()) {
				SQLConnection connection = conn.result();
				String query = "SELECT * " + "FROM regfechas ";
				connection.query(query, res -> {
					if (res.succeeded()) {
						routingContext.response().end(Json.encodePrettily(res.result().getRows()));
					} else {
						routingContext.response().setStatusCode(400).end("Error: " + res.cause());
					}
				});
			} else {
				routingContext.response().setStatusCode(400).end("Error: " + conn.cause());
			}
		});
	}

	private void getOneFecha(RoutingContext routingContext) {
		String paramStr = routingContext.request().getParam("idFilter");
		if (paramStr != null) {
			try {
				int param = Integer.parseInt(paramStr);
				mySQLClient.getConnection(conn -> {
					if (conn.succeeded()) {
						SQLConnection connection = conn.result();
						String query = "SELECT id, idSensor, valor, fecha " + "FROM regFechas " + "WHERE id = ?";
						JsonArray paramQuery = new JsonArray().add(param);
						connection.queryWithParams(query, paramQuery, res -> {
							if (res.succeeded()) {
								routingContext.response().end(Json.encodePrettily(res.result().getRows().get(0)));
							} else {
								routingContext.response().setStatusCode(400).end("Error: " + res.cause());
							}
						});
					} else {
						routingContext.response().setStatusCode(400).end("Error: " + conn.cause());
					}
				});
			} catch (ClassCastException e) {
				routingContext.response().setStatusCode(400).end();
			}
		} else {
			routingContext.response().setStatusCode(400).end();
		}
	}

	private void putElementFechas(RoutingContext routingContext) {
		RegFechas fecha = new RegFechas();
		RegFechas state = Json.decodeValue(routingContext.getBodyAsString(), RegFechas.class);
		mySQLClient.getConnection(conn -> {
			if (conn.succeeded()) {
				SQLConnection connection = conn.result();
				String query = "INSERT INTO regfechas (idSensor, valor, fecha) VALUES ('" + state.getidSensor() + "','"
						+ state.getValor() + "','" + fecha.getFecha() + "')";
				connection.query(query, res -> {
					if (res.succeeded()) {
						routingContext.response().end(Json.encodePrettily(res.result().getRows()));
					} else {
						routingContext.response().setStatusCode(400).end("Error: " + res.cause());
					}
				});
			} else {
				routingContext.response().setStatusCode(400).end("Error: " + conn.cause());
			}
		});
	}

	private void deleteElementFechas(RoutingContext routingContext) {
		String paramStr = routingContext.request().getParam("idDelete");
		if (paramStr != null) {
			try {
				int param = Integer.parseInt(paramStr);
				mySQLClient.getConnection(conn -> {
					if (conn.succeeded()) {
						SQLConnection connection = conn.result();
						String query = "DELETE FROM regfechas " + "WHERE id = ?";
						JsonArray paramQuery = new JsonArray().add(param);
						connection.queryWithParams(query, paramQuery, res -> {
							if (res.succeeded()) {
								routingContext.response().end(Json.encodePrettily(res.result().getRows()));
							} else {
								routingContext.response().setStatusCode(400).end("Error: " + res.cause());
							}
						});
					} else {
						routingContext.response().setStatusCode(400).end("Error: " + conn.cause());
					}
				});
			} catch (ClassCastException e) {
				routingContext.response().setStatusCode(400).end();
			}
		} else {
			routingContext.response().setStatusCode(400).end();
		}
	}

	// Operaciones CRUD de modWifi
	private void getAllMods(RoutingContext routingContext) {
		mySQLClient.getConnection(conn -> {
			if (conn.succeeded()) {
				SQLConnection connection = conn.result();
				String query = "SELECT * " + "FROM modWifi ";
				connection.query(query, res -> {
					if (res.succeeded()) {
						routingContext.response().end(Json.encodePrettily(res.result().getRows()));
					} else {
						routingContext.response().setStatusCode(400).end("Error: " + res.cause());
					}
				});
			} else {
				routingContext.response().setStatusCode(400).end("Error: " + conn.cause());
			}
		});
	}

	private void getOneMod(RoutingContext routingContext) {
		String paramStr = routingContext.request().getParam("idFilter");
		if (paramStr != null) {
			try {
				mySQLClient.getConnection(conn -> {
					if (conn.succeeded()) {
						SQLConnection connection = conn.result();
						String query = "SELECT idWifi, descripcion " + "FROM modWifi " + "WHERE idWifi = ?";
						JsonArray paramQuery = new JsonArray().add(paramStr);
						connection.queryWithParams(query, paramQuery, res -> {
							if (res.succeeded()) {
								routingContext.response().end(Json.encodePrettily(res.result().getRows().get(0)));
							} else {
								routingContext.response().setStatusCode(400).end("Error: " + res.cause());
							}
						});
					} else {
						routingContext.response().setStatusCode(400).end("Error: " + conn.cause());
					}
				});
			} catch (ClassCastException e) {
				routingContext.response().setStatusCode(400).end();
			}
		} else {
			routingContext.response().setStatusCode(400).end();
		}
	}

	private void putElementMod(RoutingContext routingContext) {
		ModWifi state = Json.decodeValue(routingContext.getBodyAsString(), ModWifi.class);
		mySQLClient.getConnection(conn -> {
			if (conn.succeeded()) {
				SQLConnection connection = conn.result();
				String query = "INSERT INTO modWifi (idWifi, descripcion) VALUES ('" + state.getIdWifi() + "','"
						+ state.getDescripcion() + "')";
				connection.query(query, res -> {
					if (res.succeeded()) {
						routingContext.response().end(Json.encodePrettily(res.result().getRows()));
					} else {
						routingContext.response().setStatusCode(400).end("Error: " + res.cause());
					}
				});
			} else {
				routingContext.response().setStatusCode(400).end("Error: " + conn.cause());
			}
		});
	}

	private void deleteElementMod(RoutingContext routingContext) {
		String paramStr = routingContext.request().getParam("idDelete");
		if (paramStr != null) {
			try {
				mySQLClient.getConnection(conn -> {
					if (conn.succeeded()) {
						SQLConnection connection = conn.result();
						String query = "DELETE FROM modWifi " + "WHERE idWifi = ?";
						JsonArray paramQuery = new JsonArray().add(paramStr);
						connection.queryWithParams(query, paramQuery, res -> {
							if (res.succeeded()) {
								routingContext.response().end(Json.encodePrettily(res.result().getRows()));
							} else {
								routingContext.response().setStatusCode(400).end("Error: " + res.cause());
							}
						});
					} else {
						routingContext.response().setStatusCode(400).end("Error: " + conn.cause());
					}
				});
			} catch (ClassCastException e) {
				routingContext.response().setStatusCode(400).end();
			}
		} else {
			routingContext.response().setStatusCode(400).end();
		}
	}

	// Operaciones CRUD de elementos
	private void getAllElem(RoutingContext routingContext) {
		mySQLClient.getConnection(conn -> {
			if (conn.succeeded()) {
				SQLConnection connection = conn.result();
				String query = "SELECT * " + "FROM elementos ";
				connection.query(query, res -> {
					if (res.succeeded()) {
						routingContext.response().end(Json.encodePrettily(res.result().getRows()));
					} else {
						routingContext.response().setStatusCode(400).end("Error: " + res.cause());
					}
				});
			} else {
				routingContext.response().setStatusCode(400).end("Error: " + conn.cause());
			}
		});
	}

	private void getOneElem(RoutingContext routingContext) {
		String paramStr = routingContext.request().getParam("idFilter");
		if (paramStr != null) {
			try {
				mySQLClient.getConnection(conn -> {
					if (conn.succeeded()) {
						SQLConnection connection = conn.result();
						String query = "SELECT idElemento, estado, idWifiElem " + "FROM elementos "
								+ "WHERE idElemento = ?";
						JsonArray paramQuery = new JsonArray().add(paramStr);
						connection.queryWithParams(query, paramQuery, res -> {
							if (res.succeeded()) {
								routingContext.response().end(Json.encodePrettily(res.result().getRows().get(0)));
							} else {
								routingContext.response().setStatusCode(400).end("Error: " + res.cause());
							}
						});
					} else {
						routingContext.response().setStatusCode(400).end("Error: " + conn.cause());
					}
				});
			} catch (ClassCastException e) {
				routingContext.response().setStatusCode(400).end();
			}
		} else {
			routingContext.response().setStatusCode(400).end();
		}
	}

	private void putElem(RoutingContext routingContext) {
		Elementos state = Json.decodeValue(routingContext.getBodyAsString(), Elementos.class);
		mySQLClient.getConnection(conn -> {
			if (conn.succeeded()) {
				SQLConnection connection = conn.result();
				String query = "INSERT INTO elementos (idElemento, estado, idWifiElem) VALUES ('"
						+ state.getIdElemento() + "','" + state.isEstado() + "','" + state.getIdWifiElem() + "')";
				connection.query(query, res -> {
					if (res.succeeded()) {
						routingContext.response().end(Json.encodePrettily(res.result().getRows()));
					} else {
						routingContext.response().setStatusCode(400).end("Error: " + res.cause());
					}
				});
			} else {
				routingContext.response().setStatusCode(400).end("Error: " + conn.cause());
			}
		});
	}

	private void deleteElem(RoutingContext routingContext) {
		String paramStr = routingContext.request().getParam("idDelete");
		if (paramStr != null) {
			try {
				mySQLClient.getConnection(conn -> {
					if (conn.succeeded()) {
						SQLConnection connection = conn.result();
						String query = "DELETE FROM elementos " + "WHERE idElemento = ?";
						JsonArray paramQuery = new JsonArray().add(paramStr);
						connection.queryWithParams(query, paramQuery, res -> {
							if (res.succeeded()) {
								routingContext.response().end(Json.encodePrettily(res.result().getRows()));
							} else {
								routingContext.response().setStatusCode(400).end("Error: " + res.cause());
							}
						});
					} else {
						routingContext.response().setStatusCode(400).end("Error: " + conn.cause());
					}
				});
			} catch (ClassCastException e) {
				routingContext.response().setStatusCode(400).end();
			}
		} else {
			routingContext.response().setStatusCode(400).end();
		}
	}

	/**
	 * Obtener el estado de toldo/persiana.
	 * 
	 * @param routingContext
	 */
	private void getState(RoutingContext routingContext) {
		String paramStr = routingContext.request().getParam("idFilter");
		if (paramStr != null) {
			try {
				mySQLClient.getConnection(conn -> {
					if (conn.succeeded()) {
						SQLConnection connection = conn.result();
						String query = "SELECT idElemento, estado, idWifiElem " + "FROM elementos "
								+ "WHERE idElemento = ?";
						JsonArray paramQuery = new JsonArray().add(paramStr);
						connection.queryWithParams(query, paramQuery, res -> {
							if (res.succeeded()) {
								JsonObject objeto = res.result().getRows().get(0);
								Object id = objeto.getValue("idElemento");
								Object estado = objeto.getValue("estado");
								Object pEstado = parsearEstado(estado);
								String nodeMCU = "in" + objeto.getValue("idWifiElem").toString();
								// Cerrar persiana por luminosidad.
								mqttClient.publish(nodeMCU, Buffer.buffer("estado=" + estado.toString()),
										MqttQoS.AT_LEAST_ONCE, false, false);
								routingContext.response()
										.end(Json.encodePrettily("El elemento: " + id + "--> esta: " + pEstado + "."));
							} else {
								routingContext.response().setStatusCode(400).end("Error: " + res.cause());
							}
						});
					} else {
						routingContext.response().setStatusCode(400).end("Error: " + conn.cause());
					}
				});
			} catch (ClassCastException e) {
				routingContext.response().setStatusCode(400).end();
			}
		} else {
			routingContext.response().setStatusCode(400).end();
		}
	}

	/**
	 * Modificar el estado de persiana/toldo
	 * 
	 * @param routingContext
	 */
	private void updateState(RoutingContext routingContext) {
		Elementos state = Json.decodeValue(routingContext.getBodyAsString(), Elementos.class);
		try {
			mySQLClient.getConnection(conn -> {
				if (conn.succeeded()) {
					SQLConnection connection = conn.result();
					String query = "UPDATE elementos SET estado = '" + state.isEstado() + "'" + "WHERE idElemento = '"
							+ state.getIdElemento() + "'";
					connection.query(query, res -> {
						if (res.succeeded()) {
							routingContext.response().end(Json.encodePrettily("Estado modificado."));
						} else {
							routingContext.response().setStatusCode(400).end("Error: " + res.cause());
						}
					});
				} else {
					routingContext.response().setStatusCode(400).end("Error: " + conn.cause());
				}
			});
		} catch (ClassCastException e) {
			routingContext.response().setStatusCode(400).end();
		}

	}

	// Método auxiliar para mostrar el estado de un elemento.
	private String parsearEstado(Object o) {
		String res = "";
		if (o.toString().equals("0")) {
			res = "CERRADO";
		} else {
			res = "ABIERTO";
		}
		return res;
	}

}
