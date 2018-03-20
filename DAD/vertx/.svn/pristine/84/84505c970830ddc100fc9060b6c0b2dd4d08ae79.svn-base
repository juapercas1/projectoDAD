package pruebas;

import io.vertx.core.AbstractVerticle;
import io.vertx.core.Future;

public class DadPrueba2 extends AbstractVerticle{

	public void start(Future<Void> startFuture) {
		System.out.println("Dad prueba 2 lanzado");
		startFuture.complete();
		
		vertx.eventBus().consumer("mensaje-punto-a-punto", 
				msg -> {
					System.out.println(msg.body());
					msg.reply("Sí, estoy aquí", reply -> {
						System.out.println(reply.result().body());
					});
				});
		vertx.eventBus().consumer("mensaje-broadcast",
				msg -> {
					System.out.println(msg.body());
					msg.reply("hola");
				});
	}
}
