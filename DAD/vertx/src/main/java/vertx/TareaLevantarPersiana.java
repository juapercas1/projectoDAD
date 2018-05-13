package vertx;

import org.quartz.Job;
import org.quartz.JobExecutionContext;
import org.quartz.JobExecutionException;

public class TareaLevantarPersiana implements Job {

	public void execute(JobExecutionContext jobExecutionContext) throws JobExecutionException {
		RestEP.abrirPersianaLuminosidad("PERSIANA1");
	}
}
