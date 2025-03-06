def get_all_presets(self):
    """
    Get all presets from GenieACS.

    Returns:
        dict: A dictionary containing all presets.

    Raises:
        HTTPError: If the request to GenieACS fails.
        ConnectionError: If there is a connection error.
        JSONDecodeError: If the received JSON is incorrectly formatted.

    """
    logging.info("Getting all presets from GenieACS.")
    try:
        preset_get_request = requests.get(f"{self.url}/presets/")
        preset_get_request.raise_for_status()
    except (HTTPError, ConnectionError) as e:
        logging.error(f"Failed to get provisions from GenieACS due to {e}.")
        raise

    try:
        parsed_preset_json = preset_get_request.json()
    except JSONDecodeError as e:
        msg = f"Received JSON is incorrectly formatted. Received data: {preset_get_request.text}. Exception message: {e}."
        logging.error(msg)
        raise

    logging.info(
        f"Get preset request successful. Status code {preset_get_request.status_code}"
    )
    return parsed_preset_json
