package uk.co.msaqib.zumocontrols.Interfaces

interface IServiceCallbacks{
    fun onDataReceived(byteArray: ByteArray)
    fun onError(e: String)
    fun updateStatus(status: Boolean,s: String)
}