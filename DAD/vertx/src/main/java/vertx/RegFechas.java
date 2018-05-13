package vertx;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;

public class RegFechas {

	private int id;
	private String idSensor;
	private String valor;
	private String fecha;
	private static int contador = 0; // Se utiliza para el autoincremento.

	//Constructor sin parámetros
	public RegFechas() {
		contador++;
		this.id = contador;
		//Fecha actual
		Date fechaActual = new Date();
		//Formateando la fecha:
        DateFormat formatoHora = new SimpleDateFormat("HH:mm:ss");
        DateFormat formatoFecha = new SimpleDateFormat("dd/MM/yyyy");
        String fechaFormateada = formatoFecha.format(fechaActual) +"-"+ formatoHora.format(fechaActual);
		this.fecha = fechaFormateada;
	}

	// Constructor con parámetros
	@JsonCreator
	public RegFechas(@JsonProperty("idSensor") String idSensor,@JsonProperty("valor") String valor) {
		super();
		contador++;
		this.id = contador;
		this.idSensor = idSensor;
		this.valor = valor;
		//Fecha actual
		Date fechaActual = new Date();
		//Formateando la fecha:
        DateFormat formatoHora = new SimpleDateFormat("HH:mm:ss");
        DateFormat formatoFecha = new SimpleDateFormat("dd/MM/yyyy");
        String fechaFormateada = formatoFecha.format(fechaActual) +"-"+ formatoHora.format(fechaActual);
		this.fecha = fechaFormateada;
	}

	// Guetters and Setters
	public Integer getId() {
		return id;
	}

	public String getidSensor() {
		return idSensor;
	}

	public void setidSensor(String idSensor) {
		this.idSensor = idSensor;
	}

	public String getValor() {
		return valor;
	}

	public void setValor(String valor) {
		this.valor = valor;
	}

	public String getFecha() {
		return fecha;
	}


	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((fecha == null) ? 0 : fecha.hashCode());
		result = prime * result + id;
		result = prime * result + ((idSensor == null) ? 0 : idSensor.hashCode());
		result = prime * result + ((valor == null) ? 0 : valor.hashCode());
		return result;
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj)
			return true;
		if (obj == null)
			return false;
		if (getClass() != obj.getClass())
			return false;
		RegFechas other = (RegFechas) obj;
		if (fecha == null) {
			if (other.fecha != null)
				return false;
		} else if (!fecha.equals(other.fecha))
			return false;
		if (id != other.id)
			return false;
		if (idSensor == null) {
			if (other.idSensor != null)
				return false;
		} else if (!idSensor.equals(other.idSensor))
			return false;
		if (valor == null) {
			if (other.valor != null)
				return false;
		} else if (!valor.equals(other.valor))
			return false;
		return true;
	}

	@Override
	public String toString() {
		return "RegFechas [id=" + id + ", idSensor=" + idSensor + ", valor=" + valor
				+ ", fecha=" + fecha + "]";
	}

}
