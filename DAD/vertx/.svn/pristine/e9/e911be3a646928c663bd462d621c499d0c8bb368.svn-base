package vertx;

public class Sensores {
	private String idCodigo;
	private String tipoSensor;
	private String descripcion;
	
	//Constructor vacio
	public Sensores() {
		this("","","");
	}

	//Constructor con parámetros
	public Sensores(String idCodigo, String tipoSensor, String descripcion) {
		super();
		this.idCodigo = idCodigo;
		this.tipoSensor = tipoSensor;
		this.descripcion = descripcion;
	}
	

	//Métodos guetters and Setters
	public String getIdCodigo() {
		return idCodigo;
	}

	public void setIdCodigo(String idCodigo) {
		this.idCodigo = idCodigo;
	}

	public String getTipoSensor() {
		return tipoSensor;
	}

	public void setTipoSensor(String tipoSensor) {
		this.tipoSensor = tipoSensor;
	}

	public String getDescripcion() {
		return descripcion;
	}

	public void setDescripcion(String descripcion) {
		this.descripcion = descripcion;
	}

	
	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result + ((descripcion == null) ? 0 : descripcion.hashCode());
		result = prime * result + ((idCodigo == null) ? 0 : idCodigo.hashCode());
		result = prime * result + ((tipoSensor == null) ? 0 : tipoSensor.hashCode());
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
		Sensores other = (Sensores) obj;
		if (descripcion == null) {
			if (other.descripcion != null)
				return false;
		} else if (!descripcion.equals(other.descripcion))
			return false;
		if (idCodigo == null) {
			if (other.idCodigo != null)
				return false;
		} else if (!idCodigo.equals(other.idCodigo))
			return false;
		if (tipoSensor == null) {
			if (other.tipoSensor != null)
				return false;
		} else if (!tipoSensor.equals(other.tipoSensor))
			return false;
		return true;
	}

	@Override
	public String toString() {
		return "Sensores [idCodigo=" + idCodigo + ", tipoSensor=" + tipoSensor + ", descripcion=" + descripcion + "]";
	}
	
	

}
