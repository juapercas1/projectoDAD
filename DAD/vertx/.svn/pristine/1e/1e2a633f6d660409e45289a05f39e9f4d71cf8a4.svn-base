package vertx;

public class ModWifi {
	private String idWifi;
	private String descripcion;
	
	
	public ModWifi() {
		this("","");
	}
	
	public ModWifi(String idWifi, String descripcion) {
		super();
		this.idWifi = idWifi;
		this.descripcion = descripcion;
	}

	public String getIdWifi() {
		return idWifi;
	}

	public void setIdWifi(String idWifi) {
		this.idWifi = idWifi;
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
		result = prime * result + ((idWifi == null) ? 0 : idWifi.hashCode());
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
		ModWifi other = (ModWifi) obj;
		if (descripcion == null) {
			if (other.descripcion != null)
				return false;
		} else if (!descripcion.equals(other.descripcion))
			return false;
		if (idWifi == null) {
			if (other.idWifi != null)
				return false;
		} else if (!idWifi.equals(other.idWifi))
			return false;
		return true;
	}

	@Override
	public String toString() {
		return "ModWifi [idWifi=" + idWifi + ", descripcion=" + descripcion + "]";
	}
	
	
}
